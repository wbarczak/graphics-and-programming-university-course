#include <fstream>
#include <exception>

#include <nlohmann/json.hpp>

#include "Game.hpp"

namespace nl = nlohmann;

Game::Game()
{
	initPrefabGenerators();

	recursiveLoad("resources/textures", [&](const auto& path, const auto& name)
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

	scenes.registerScene("MainMenu", [&]() { return std::make_unique<Scenes::MainMenu>(apiScene, window); });
	scenes.registerScene("LevelEditor", [&]() { return std::make_unique<Scenes::LevelEditor>(apiScene, m_ctx); });
	scenes.registerScene("Platforming", [&]() { return std::make_unique<Scenes::Platforming>(apiScene, m_ctx); });
	scenes.registerScene("Pause", [&]() { return std::make_unique<Scenes::Pause>(apiScene, window); });
	scenes.registerScene("Settings", [&]() { return std::make_unique<Scenes::Settings>(apiScene, mapping, window); });
	scenes.push("MainMenu");

	mapping.set("Jump", px::InputId::Space);
	mapping.set("Left", px::InputId::A);
	mapping.set("Right", px::InputId::D);
	mapping.set("Up", px::InputId::W);
	mapping.set("Down", px::InputId::S);
	mapping.set("Confirm", px::InputId::Space);
	mapping.set("Pause", px::InputId::Escape);

	m_ctx.tiles["empty"] = Tile{ Tile::Type::Air, "", "empty" };
	m_ctx.tiles["solid_block"] = Tile{ Tile::Type::Solid, "solid_block", "solid_block" };

	assets.tileSprites.set("solid_block", px::TileSprite{ "solid_block" });

	loadSprites();

	recursiveLoad("resources/entities", [&](const auto& path, const auto& name)
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
			{ assets.textures.get("background/1"), 0.0625f },
			{ assets.textures.get("background/2"), 0.125f },
			{ assets.textures.get("background/4"), 0.25f },
			{ assets.textures.get("background/3"), 0.5f },
			{ assets.textures.get("background/5"), 1.0f }
		}
	);

	assets.backgrounds.set("background", std::move(background));

	assets.font = sf::Font("resources/Butterpop.otf");
	
	EntityPrefab cloudParticle;
	cloudParticle.emplace<Transform>();
	cloudParticle.emplace<Lifetime>(sf::Time::Zero, sf::milliseconds(400));
	cloudParticle.emplace<IsParticle>();
	cloudParticle.emplace<px::Animation>(assets.clipMaps.get("particle"));
	m_ctx.entities.set("cloud_particle", std::move(cloudParticle));
}

void Game::loadSprites()
{
	recursiveLoad("resources/sprites", [&](const auto& path, const auto& name)
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
	m_prefabGenerators.emplace("tile_hazard", [&](const auto& obj)
	{
		EntityPrefab prefab;
		prefab.emplace<Stationary>();
		prefab.emplace<Hitbox>(sf::FloatRect{ {0.25f, 0.25f}, {0.5f, 0.5f} }, ColiderType::Hazard);
		prefab.emplace<px::Animation>(assets.clipMaps.get(obj["sprite"]));
		if (obj.contains("toggle") && obj["toggle"] == true)
		{
			prefab.emplace<Toggle>();
		}
		return prefab;
	});

	m_prefabGenerators.emplace("tile_platform", [&](const auto& obj)
	{
		EntityPrefab prefab;
		prefab.emplace<Stationary>();
		prefab.emplace<Hitbox>(sf::FloatRect{ {0.f, 0.f}, {1.f, 0.2f} }, ColiderType::Platform);
		prefab.emplace<px::Animation>(assets.clipMaps.get(obj["sprite"]));
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
}