#include <fstream>
#include <exception>

#include <nlohmann/json.hpp>

#include "Game.hpp"

#include <windows.h>

namespace nl = nlohmann;

Game::Game()
{
	initPrefabGenerators();

	m_music.openFromFile(RESOURCES "Etirwer (Looped).ogg");
	m_music.setLooping(true);

	recursiveLoad(RESOURCES "textures", [&](const auto& path, const auto& name)
	{
		sf::Texture texture;
		if (!texture.loadFromFile(path))
		{
			return;
		}
		texture.setRepeated(true);

		assets.textures.set(name, std::move(texture));

		SPDLOG_INFO("Texture loaded: {}", name);
	});

	recursiveLoad(RESOURCES "sounds", [&](const auto& path, const auto& name)
	{
		sf::SoundBuffer sound;
		if (!sound.loadFromFile(path))
			{
			return;
		}

		m_ctx.sounds.insert({ name, std::move(sound) });

		SPDLOG_INFO("Sound loaded: {}", name);
	});

	scenes.registerScene("MainMenu", [&]() { return std::make_unique<Scenes::MainMenu>(apiScene, window, m_ctx); });
	scenes.registerScene("LevelEditor", [&]() { return std::make_unique<Scenes::LevelEditor>(apiScene, m_ctx); });
	scenes.registerScene("Platforming", [&]() { return std::make_unique<Scenes::Platforming>(apiScene, m_ctx); });
	scenes.registerScene("Pause", [&]() { return std::make_unique<Scenes::Pause>(apiScene, window); });
	scenes.registerScene("Settings", [&]() { return std::make_unique<Scenes::Settings>(apiScene, mapping, window); });
	scenes.push("MainMenu");

	mapping.set("Jump", px::InputId::Space);
	mapping.set("Left", px::InputId::A);
	mapping.set("Right", px::InputId::D);
	mapping.set("Interact", px::InputId::E);

	m_ctx.tiles["empty"] = Tile{ Tile::Type::Air, "", "empty" };
	m_ctx.tiles["dirt"] = Tile{ Tile::Type::Solid, "dirt", "dirt" };
	m_ctx.tiles["moss"] = Tile{ Tile::Type::Solid, "moss", "moss" };
	m_ctx.tiles["cobble"] = Tile{ Tile::Type::Solid, "cobble", "cobble" };

	assets.tileSprites.set("dirt", px::TileSprite("tiles/dirt"));
	assets.tileSprites.set("cobble", px::TileSprite("tiles/cobble_tileset"));
	assets.tileSprites.set("moss", px::TileSprite("tiles/moss_on_cobble_tileset"));

	loadSprites();

	{
		nl::json json;
		std::ifstream file("settings.json");

		bool loaded = file.is_open();

		if (file.is_open())
		{
			try
			{
				json << file;
			}
			catch (const std::exception& e)
			{
				loaded = false;
			}
		}

		std::unordered_map<std::string, px::InputId> strToId;
		for (size_t i{}; i < px::k_allButtonsCount; ++i)
		{
			auto id = static_cast<px::InputId>(i);
			strToId.insert({ px::stringifyInputId(id), id });
		}

		sf::Listener::setGlobalVolume(loaded ? json["volume"].get<float>() : 100.f);
		mapping.set("Left", !loaded? px::InputId::A : strToId.at(json["actions"]["Left"]));
		mapping.set("Right", !loaded? px::InputId::D : strToId.at(json["actions"]["Right"]));
		mapping.set("Jump", !loaded? px::InputId::Space : strToId.at(json["actions"]["Jump"]));
		mapping.set("Interact", !loaded? px::InputId::E : strToId.at(json["actions"]["Interact"]));
	}

	recursiveLoad(RESOURCES "entities", [&](const auto& path, const auto& name)
	{
		nl::json json;
		std::ifstream file(path);

		try
		{
			json << file;
		}
		catch (const std::exception& e)
		{
			SPDLOG_ERROR("{}", e.what());
			return;
		}

		m_ctx.entities.set(name, m_prefabGenerators.at(json["type"])(json));

		SPDLOG_INFO("Prefab loaded: {}", name);
	});

	px::BackgroundData background(
		{
			{ assets.textures.get("background/0"), 0.03125f },
			{ assets.textures.get("background/1"), 0.0625f, -.12f },
			{ assets.textures.get("background/2"), 0.125f },
			{ assets.textures.get("background/4"), 0.25f },
			{ assets.textures.get("background/3"), 0.5f, -1.f },
			{ assets.textures.get("background/5"), 1.0f }
		}
	);

	assets.backgrounds.set("background", std::move(background));

	px::BackgroundData settingsBackground(
		{
			{ assets.textures.get("settings_bg"), 0.f }
		}
	);

	assets.backgrounds.set("settings", std::move(settingsBackground));

	assets.font = sf::Font(RESOURCES "Butterpop.otf");

	auto desktop = sf::VideoMode::getDesktopMode();
	window.create(sf::VideoMode(desktop.size), "Nellie's Adventure", sf::Style::None);
	window.setVerticalSyncEnabled(true);
	window.setKeyRepeatEnabled(false);
	m_music.play();
}

void Game::loadSprites()
{
	recursiveLoad(RESOURCES "sprites", [&](const auto& path, const auto& name)
	{
		nl::json json;
		std::ifstream file(path);

		try
		{
			json << file;
		}
		catch (const std::exception& e)
		{
			SPDLOG_ERROR("{}", e.what());
			return;
		}

		const sf::Texture& texture = assets.textures.get(json["texture"]);
		sf::Vector2f origin{ static_cast<float>(json["origin"][0]), static_cast<float>(json["origin"][1]) };
		std::string defaultClip = json["default"];
		sf::Vector2i tile = static_cast<sf::Vector2i>(texture.getSize());
		tile.x /= json["grid"][0];
		tile.y /= json["grid"][1];

		px::AnimationClipMap clips;
		clips.fallback = defaultClip;
		for (const auto& [key, value] : json["clips"].items())
		{
			sf::Time time = sf::seconds(value["duration"]);

			std::string typeString = value["type"];
			px::AnimationClipType type = px::AnimationClipType::Normal;
			if (typeString == "looped")
			{
				type = px::AnimationClipType::Looped;
			}
			if (typeString == "sticky")
			{
				type = px::AnimationClipType::Sticky;
			}

			std::vector<px::AnimationFrame> frames;
			for (const auto& vector : value["frames"])
			{
				sf::IntRect frameRect{ {vector[0] * tile.x, vector[1] * tile.y}, tile };
				frames.push_back({ frameRect, time });
			}

			px::AnimationClip clip{ texture };
			clip.frames = std::move(frames);
			clip.origin = origin;
			clip.type = type;

			clips.map.insert({ key, std::move(clip)});
		}

		assets.clipMaps.set(name, std::move(clips));

		SPDLOG_INFO("Sprite loaded: {}", name);
	});
}

void Game::initPrefabGenerators()
{
	m_prefabGenerators.emplace("tile", [&](const auto& obj)
	{
		EntityPrefab prefab;
		prefab.emplace<Stationary>();
		if (obj["colider"] == "platform")
		{
			prefab.emplace<Hitbox>(sf::FloatRect{ {0.f, 0.f}, {1.f, 0.2f} }, ColiderType::Platform);
		}
		else if (obj["colider"] == "hazard")
		{
			prefab.emplace<Hitbox>(sf::FloatRect{ {0.15f, 0.15f}, {0.7f, 0.7f} }, ColiderType::Hazard);
		}
		else if (obj["colider"] == "solid")
		{
			prefab.emplace<Hitbox>(sf::FloatRect{ {0.f,0.f}, {1.f,1.f} }, ColiderType::Solid);
		}
		else if (obj["colider"] == "zone")
		{
			prefab.emplace<Hitbox>(sf::FloatRect{ {0.f,0.f}, {1.f,1.f} }, ColiderType::Zone);
		}
		prefab.emplace<px::Animation>(assets.clipMaps.get(obj["sprite"]));
		if (obj.contains("crumbling") && obj["crumbling"] == true)
		{
			Crumbling crumbling;
			crumbling.onTime = sf::milliseconds(500);
			crumbling.offTime = sf::seconds(5);
			prefab.emplace<Crumbling>(crumbling);
		}
		else if (obj.contains("toggle") && obj["toggle"] == true)
		{
			prefab.emplace<Toggle>();
		}
		if (obj.contains("trampoline") && obj["trampoline"] == true)
		{
			prefab.emplace<Trampoline>();
		}
		if (obj.contains("trigger") && obj["trigger"] == true)
		{
			prefab.emplace<Trigger>();
		}
		return prefab;
	});

	m_prefabGenerators.emplace("actor", [&](const auto& obj)
	{
		EntityPrefab prefab;
		prefab.emplace<Transform>();
		prefab.emplace<Hitbox>(sf::FloatRect{ {-.25f, -.75f}, {.5f, .75f} }, ColiderType::Physics);
		prefab.emplace<px::Animation>(assets.clipMaps.get(obj["sprite"]));
		prefab.emplace<Controllable>();
		return prefab;
	});

	m_prefabGenerators.emplace("particle", [&](const auto& obj)
	{
		EntityPrefab prefab;
		prefab.emplace<Transform>();
		prefab.emplace<px::Animation>(assets.clipMaps.get(obj["sprite"]));
		Lifetime lifetime;
		lifetime.max = sf::seconds(obj["lifetime"]);
		prefab.emplace<Lifetime>(lifetime);
		return prefab;
	});
}