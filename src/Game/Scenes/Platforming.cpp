#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <optional>
#include <fstream>

#include <nlohmann/json.hpp>

#include "Platforming.hpp"
#include "Game/Constants.hpp"
#include "Game/DeprecatedDevice.hpp"
#include "../Level.hpp"

namespace nl = nlohmann;

struct PairHash
{
	template <typename T1, typename T2>
	size_t operator()(const std::pair<T1, T2>& p) const {
		auto h1 = std::hash<T1>{}(p.first);
		auto h2 = std::hash<T2>{}(p.second);
		return h1 ^ (h2 << 1);
	}
};

Scenes::Platforming::Platforming(px::SceneInitCtx& ctx, Context& gctx) :
	Scene(ctx),
	m_ctx(gctx),
	m_map(sf::Vector2u(60, 12), m_ctx.tiles["empty"]),
	m_jump(m_ctx.sounds.at("jump")),
	m_landing(m_ctx.sounds.at("landing")),
	m_step(m_ctx.sounds.at("step"))
{
	std::ifstream file(RESOURCES + ("levels/" + std::to_string(m_ctx.selectedLevel)) + ".json");
	nl::json obj;
	
	file >> obj;

	sf::Vector2u size{};
	auto mapBuilder = obj["map"].get<std::vector<std::string>>();
	size.y = uint32_t(mapBuilder.size());
	size.x = std::min_element(mapBuilder.begin(), mapBuilder.end(), [](const std::string& l, const std::string& r) {return l.size() < r.size(); })->size();

	m_map.resize(size);

	std::unordered_map<std::pair<int32_t, int32_t>, entt::entity, PairHash> tileEntities;

	for (uint32_t y = 0; y < m_map.size().y; ++y)
	{
		for (uint32_t x = 0; x < m_map.size().x; ++x)
		{
			if (m_ctx.tempMapping.count(mapBuilder[y][x]))
			{
				auto& [name, isEntity] = m_ctx.tempMapping.at(mapBuilder[y][x]);

				if (!isEntity)
				{
					m_map.at({ x, y }) = m_ctx.tiles.at(name);
				}
				else
				{
					auto entity = m_ctx.entities.get(name).spawn(m_registry);
					auto& stationary =  m_registry.get<Stationary>(entity);
					stationary.position = { float(x), float(y) };
					tileEntities.insert({ {x, y}, entity });
				}
			}
		}
	}

	for (const auto& prefab : obj["devices"])
	{
		DeprecatedDeviceType type;

		if (prefab["type"] == "time")
		{
			type = DeprecatedDeviceType::Timed;
		}
		else
		{
			type = DeprecatedDeviceType::And;
		}

		DeprecatedDevice device{type};

		if (type == DeprecatedDeviceType::Timed)
		{
			device.onTime = sf::seconds(prefab["on"]);
			device.offTime = sf::seconds(prefab["off"]);
		}
		else
		{
			for (auto e : prefab["in"])
			{
				std::pair pair{ e[0], e[1] };

				if (!tileEntities.count(pair))
				{
					continue;
				}

				device.in.push_back({ tileEntities.at(pair), e.size() > 2 ? e[2].get<bool>() : false });
			}
		}

		for (auto e : prefab["out"])
		{
			std::pair pair{ e[0], e[1] };

			if (!tileEntities.count(pair))
			{
				continue;
			}

			device.out.push_back({ tileEntities.at(pair), e.size() > 2 ? e[2].get<bool>() : false });
		}

		m_devices.push_back(std::move(device));
	}

	{
		auto player = m_ctx.entities.get("player").spawn(m_registry);
		auto& transform = m_registry.get<Transform>(player);
		transform.pos.x = obj["player"][0];
		transform.pos.y = obj["player"][1];
		transform.oldPos = transform.pos;
		m_cameraPosition = transform.pos;
		m_oldCameraPosition = transform.pos;
	}

	auto mapSize = static_cast<sf::Vector2f>(m_map.size());

	m_bounds.push_back(sf::FloatRect{ {-1.f,-1.f},{mapSize.x + 2.f, 1.f} });
	m_bounds.push_back(sf::FloatRect{ {-1.f,-1.f},{1.f, mapSize.y} });
	m_bounds.push_back(sf::FloatRect{ {-1.f,mapSize.y},{mapSize.x + 2.f, 1.f} });
	m_bounds.push_back(sf::FloatRect{ {mapSize.x,-1.f},{1.f, mapSize.y} });
}

void Scenes::Platforming::restart(px::UpdateCtx& ctx)
{
	if (m_restarting)
	{
		return;
	}

	m_restarting = true;
	ctx.transition.start([&]()
	{
		api.comms.replace("Platforming");
	});
}

void Scenes::Platforming::crumble(px::UpdateCtx& ctx)
{
	auto view = m_registry.view<Crumbling>();

	view.each([&](auto& crumbling)
	{
		if (!crumbling.active)
		{
			return;
		}

		crumbling.accumulated += ctx.dt;

		if (crumbling.accumulated <= crumbling.onTime)
		{
			return;
		}

		const auto fullCycle = crumbling.onTime + crumbling.offTime;
		if (crumbling.accumulated > fullCycle)
		{
			crumbling.active = false;
			crumbling.isAir = false;
			crumbling.accumulated = sf::Time::Zero;

			return;
		}

		crumbling.isAir = true;
	});
}

void Scenes::Platforming::update(px::UpdateCtx& ctx)
{
	m_elapsed += ctx.dt;

	if (api.mapping.isPressed(px::InputId::Escape))
	{
		api.comms.push("Pause");
	}

	animate(ctx);
}

void Scenes::Platforming::fixedUpdate(px::UpdateCtx& ctx)
{
	m_timeSinceLastStep += ctx.dt;

	crumble(ctx);

	auto view = m_registry.view<Lifetime>();
	view.each([&](entt::entity entity, Lifetime& lifetime)
	{
		lifetime.lived += ctx.dt;

		if (lifetime.lived > lifetime.max)
		{
			m_registry.destroy(entity);
		}
	});

	for (auto& device : m_devices)
	{
		switch (device.type)
		{
		case DeprecatedDeviceType::Timed:
			{
				bool state = device.accumulated % (device.onTime + device.offTime) <= device.onTime;

				for (auto& out : device.out)
				{
					if (auto* toggle = m_registry.try_get<Toggle>(out.entity))
					{
						toggle->active = out.inverted ? !state : state;
					}
				}

				device.accumulated += ctx.dt;
				break;
			}
		case DeprecatedDeviceType::And:
			{
				bool state = std::all_of(device.in.begin(), device.in.end(), [&](const auto& in)
					{
						if (const auto* trigger = m_registry.try_get<Trigger>(in.entity))
						{
							return in.inverted ? !trigger->active : trigger->active;
						}

						return false;
					});

				for (auto& out : device.out)
				{
					if (auto* toggle = m_registry.try_get<Toggle>(out.entity))
					{
						toggle->active = out.inverted ? !state : state;
					}
				}

				break;
			}
		}
	}

	playerControlSystem(ctx);

	movementAndColisionSystem(ctx);
}

void Scenes::Platforming::draw(px::DrawCtx& ctx) const
{
	ctx.window.clear(sf::Color::Blue);

	const sf::Vector2u size = m_map.size();
	const float unitPixels = api.scaling.getUnit();
	
	{
		sf::Vector2f windowSize = static_cast<sf::Vector2f>(ctx.window.getSize());
		sf::Vector2f halfScreenTiles = windowSize / static_cast<float>(unitPixels) / 2.0f;

		sf::Vector2f position = px::lerp(m_oldCameraPosition, m_cameraPosition, ctx.alpha);
		position.x = std::min(position.x, m_map.size().x - halfScreenTiles.x);
		position.y = std::max(position.y, halfScreenTiles.y);
		position.x = std::max(position.x, halfScreenTiles.x);
		position.y = std::min(position.y, m_map.size().y - halfScreenTiles.y);

		ctx.window.draw(px::Background(api.assets.backgrounds.get("background"), position.x * unitPixels, m_elapsed));

		sf::View view(
			position * static_cast<float>(unitPixels),
			static_cast<sf::Vector2f>(ctx.window.getSize())
		);

		ctx.window.setView(view);
	}

	for (size_t y = 0; y < size.y; ++y) for (size_t x = 0; x < size.x; ++x)
	{
		sf::Vector2u position(x, y);

		if (m_map.at(position).sprite != "")
		{
			auto sprite(api.assets.tileSprites.get(m_map.at(position).sprite).get(getAdjacent(m_map, position), api.assets.textures));
			sprite.setPosition(static_cast<sf::Vector2f>(position) * unitPixels);
			sprite.setScale(api.scaling.getScale());
			ctx.window.draw(sprite);
		}
	}

	auto view = m_registry.view<const px::Animation, const Transform>(entt::exclude<Controllable>);

	view.each([&](const auto& animation, const auto& transform) {
		sf::Vector2f position = px::lerp(transform.oldPos, transform.pos, ctx.alpha) * static_cast<float>(unitPixels);

		sf::Sprite sprite = animation.getSprite().value();
		sprite.setPosition(position);
		sprite.setScale(api.scaling.getScale());
		ctx.window.draw(sprite);
	});

	auto stationaryView = m_registry.view<const px::Animation, const Stationary>();
	stationaryView.each([&](const auto& animation, const auto& stationary)
	{
		sf::Vector2f position = stationary.position * static_cast<float>(unitPixels);

		sf::Sprite sprite = animation.getSprite().value();
		sprite.setPosition(position);
		sprite.setScale(api.scaling.getScale());
		ctx.window.draw(sprite);
	});

	auto playerView = m_registry.view<const px::Animation, const Transform, const Controllable>();

	playerView.each([&](const auto& animation, const auto& transform, const auto& controllable) {
		sf::Vector2f position = px::lerp(transform.oldPos, transform.pos, ctx.alpha) * static_cast<float>(unitPixels);

		sf::Sprite sprite = animation.getSprite().value();
		sprite.setPosition(position);
		sprite.setScale(api.scaling.getScale());
		ctx.window.draw(sprite);
		});

	/*auto hitboxView = m_registry.view<const Hitbox>();

	hitboxView.each([&](entt::entity entity, const Hitbox& hitbox)
	{
		sf::Vector2f worldPosition{};

		if (const auto* toggle = m_registry.try_get<Toggle>(entity))
		{
			if (!toggle->active)
			{
				return;
			}
		}
		else if (const auto* crumbling = m_registry.try_get<Crumbling>(entity))
		{
			if (crumbling->isAir)
			{
				return;
			}
		}

		if (const auto* transform = m_registry.try_get<Transform>(entity))
		{
			worldPosition = px::lerp(transform->oldPos, transform->pos, ctx.alpha) + hitbox.rect.position;
		}
		else if (const auto* stationary = m_registry.try_get<Stationary>(entity))
		{
			worldPosition = stationary->position + hitbox.rect.position;
		}
		else
		{
			return;
		}

		sf::RectangleShape hitboxRectangle{ hitbox.rect.size * unitPixels };
		sf::Vector2f position = worldPosition * static_cast<float>(unitPixels);
		hitboxRectangle.setPosition(position);
		hitboxRectangle.setFillColor(sf::Color(255, 0, 0, 150));

		ctx.window.draw(hitboxRectangle);
	});*/

	ctx.window.setView(px::getRenderTargetView(ctx.window));
	sf::RectangleShape vignette(static_cast<sf::Vector2f>(ctx.window.getSize()));
	vignette.setTexture(&api.assets.textures.get("vignette"));
	vignette.setFillColor({ 0, 0, 0, 50 });
	ctx.window.draw(vignette);
}

void Scenes::Platforming::animate(px::UpdateCtx& ctx)
{
	auto view = m_registry.view<px::Animation>();

	view.each([&](entt::entity entity, auto& animation) {
		animation.update(ctx.dt);

		if (const auto* transform = m_registry.try_get<Transform>(entity))
		{
			animation.setMirrored(transform->facing != 1);
		}
	});

	auto playerView = m_registry.view<const Controllable, const Transform, px::Animation>();

	playerView.each([&](const auto& controllable, const auto& transform, auto& animation) {
		if (!controllable.grounded)
		{
			if (transform.vel.y < 0.0f)
			{
				animation.play("jump");
				return;
			}
			animation.play("fall");
			return;
		}
		if (transform.vel.x == 0.0f)
		{
			animation.play("idle");
			return;
		}
		animation.play("run");
	});

	auto toggleView = m_registry.view<const Toggle, px::Animation>();
	toggleView.each([&](const auto& toggle, auto& animation)
	{
		if (toggle.active)
		{
			animation.play("active");
		}
		else
		{
			animation.play("inactive");
		}
	});

	auto crumblingView = m_registry.view<const Crumbling, px::Animation>();
	crumblingView.each([&](const auto& crumbling, auto& animation)
	{
		if (!crumbling.isAir)
		{
			animation.play("active");
		}
		else
		{
			animation.play("inactive");
		}
	});

	auto triggerView = m_registry.view<const Trigger, px::Animation>();

	triggerView.each([&](const auto& trigger, auto& animation)
	{
		if (trigger.active)
		{
			animation.play("active");
		}
		else
		{
			animation.play("inactive");
		}
	});
}

void Scenes::Platforming::playerControlSystem(px::UpdateCtx& ctx)
{
	auto view = m_registry.view<Controllable, Transform, const Hitbox>();

	view.each([&](auto& controllable, auto& transform, const auto& hitbox) {
		if (api.mapping.isPressed("Jump"))
		{
			controllable.jumpBuffer = sf::Time::Zero;
			transform.jumpStartY = transform.pos.y;
		}
		else
		{
			controllable.jumpBuffer += ctx.dt;
		}

		controllable.cayoteTime += ctx.dt;

		if (!controllable.wasGrounded && controllable.grounded)
		{
			m_landing.play();
		}
		
		if (controllable.grounded)
		{
			controllable.cayoteTime = sf::Time::Zero;
			controllable.canJump = true;
		}

		if (controllable.jumpBuffer <= k_bufferedJumpLimit && controllable.canJump && controllable.cayoteTime <= k_cayoteTime)
		{
			transform.vel.y = -k_jumpVelocity;
			controllable.canJump = false;
			controllable.jumpBuffer = sf::seconds(999);

			m_jump.play();
		}
		else if (transform.vel.y < 0.0f)
		{
			transform.vel.y = std::min(transform.vel.y + k_downAcceleration * ctx.dt.asSeconds(), k_maxDownAcceleration);
		}
		else
		{
			transform.vel.y = std::min(transform.vel.y + k_downAcceleration * k_fallMultiplayer * ctx.dt.asSeconds(), k_maxDownAcceleration);
		}

		int32_t direction = 0 - api.mapping.isHeld("Left") + api.mapping.isHeld("Right");

		transform.facing = direction != 0 ? direction : transform.facing;

		transform.vel.x += (direction * k_acceleration * ctx.dt.asSeconds());

		if (!direction)
		{
			float newVelocity = std::abs(transform.vel.x) - k_deceleration * ctx.dt.asSeconds();

			if (newVelocity < 0.0f)
			{
				transform.vel.x = 0.0f;
				return;
			}

			transform.vel.x = (transform.vel.x > 0.0f ? 1.0f : -1.0f) * newVelocity;
			return;
		}

		if (std::abs(transform.vel.x) > k_maxSpeed)
		{
			transform.vel.x = (transform.vel.x > 0.0f ? 1.0f : -1.0f) * k_maxSpeed;

			if (m_timeSinceLastStep > sf::milliseconds(400) && controllable.grounded)
			{
				m_timeSinceLastStep = sf::Time::Zero;
				m_step.play();
			}
		}
	});
}

void Scenes::Platforming::movementAndColisionSystem(px::UpdateCtx& ctx)
{
	struct Colider
	{
		float time;
		sf::FloatRect rectangle;
		std::optional<entt::entity> entity;

		bool operator<(const Colider& o)
		{
			return time < o.time;
		}
	};

	auto view = m_registry.view<Transform, const Hitbox>();
	auto stationaryView = m_registry.view<const Stationary, const Hitbox>();

	view.each([&](entt::entity entity, Transform& transform, const Hitbox& hitbox)
	{
		if (hitbox.type != ColiderType::Physics)
		{
			return;
		}

		if (auto* controllable = m_registry.try_get<Controllable>(entity))
		{
			controllable->wasGrounded = controllable->grounded;
			controllable->grounded = false;
		}

		transform.oldPos = transform.pos;

		sf::FloatRect entityRect = hitbox.rect;
		entityRect.position += transform.pos;
		sf::Vector2u size = m_map.size();
		sf::Vector2f deltaDistance = transform.vel * ctx.dt.asSeconds();

		static std::vector<Colider> coliders;
		coliders.clear();

		for (auto bound : m_bounds)
		{
			px::ColisionResult result = px::sweptAABB(entityRect, bound, deltaDistance);
			coliders.push_back({ result.time, bound });
		}

		for (uint32_t y{}; y < size.y; ++y)
		{
			for (uint32_t x{}; x < size.x; ++x)
			{
				if (m_map.at({ x,y }).type == Tile::Type::Air)
				{
					continue;
				}

				sf::FloatRect tileRect{ {static_cast<float>(x), static_cast<float>(y)}, {1.0f, 1.0f} };
				px::ColisionResult result = px::sweptAABB(entityRect, tileRect, deltaDistance);

				if (!result.hit)
				{
					continue;
				}

				coliders.push_back({ result.time, tileRect });
			}
		}

		stationaryView.each([&](entt::entity innerEntity, const Stationary& innerStationary, const Hitbox& innerHitbox)
		{
			if (entity == innerEntity || innerHitbox.type == ColiderType::Physics)
			{
				return;
			}

			sf::FloatRect innerEntityRect = innerHitbox.rect;
			innerEntityRect.position += innerStationary.position;
			px::ColisionResult result = px::sweptAABB(entityRect, innerEntityRect, deltaDistance);

			if (!result.hit)
			{
				return;
			}

			coliders.push_back({ result.time, innerEntityRect, innerEntity });
		});

		std::sort(coliders.begin(), coliders.end());

		for (const auto& colider : coliders)
		{
			px::ColisionResult result = px::sweptAABB(entityRect, colider.rectangle, deltaDistance);

			if (!result.hit)
			{
				continue;
			}

			ColiderType type = colider.entity ? m_registry.get<Hitbox>(*colider.entity).type : ColiderType::Solid;
			auto* toggle = colider.entity ? m_registry.try_get<Toggle>(*colider.entity) : nullptr;
			auto* crumbling = colider.entity ? m_registry.try_get<Crumbling>(*colider.entity) : nullptr;

			const bool onTop = result.normal.y < 0.f;
			const bool rightAngle = (type != ColiderType::Platform || (type == ColiderType::Platform && onTop));
			const bool toggled = !toggle || toggle->active;
			const bool crumbled = crumbling && crumbling->isAir;
			const bool resolve = rightAngle && toggled && !crumbled && type != ColiderType::Zone;

			if (!resolve)
			{
				continue;
			}

			transform.vel += sf::Vector2f{
					result.normal.x * std::abs(transform.vel.x),
					result.normal.y * std::abs(transform.vel.y)
			} *(1.0f - result.time);

			deltaDistance = transform.vel * ctx.dt.asSeconds();

			if (type == ColiderType::Hazard)
			{
				restart(ctx);
			}

			if (auto* controllable = m_registry.try_get<Controllable>(entity))
			{
				if (result.normal.y < 0.f)
				{
					controllable->grounded = true;
				}
			}

			if (!colider.entity)
			{
				continue;
			}

			if (auto * crumbling = m_registry.try_get<Crumbling>(*colider.entity))
			{
				crumbling->active = true;
			}

			if (m_registry.all_of<Trampoline>(*colider.entity) && onTop)
			{
				m_registry.get<px::Animation>(*colider.entity).play("active");

				transform.vel.y = -20.f;
				m_registry.get<Controllable>(entity).canJump = false;

				m_jump.play();
			}
		}

		transform.pos += transform.vel * ctx.dt.asSeconds();

		stationaryView.each([&](entt::entity innerEntity, const Stationary& innerStationary, const Hitbox& innerHitbox)
		{
			auto colider = innerHitbox.rect;
			colider.position += innerStationary.position;

			if (!px::colideAABB(colider, entityRect))
			{
				return;
			}

			bool active = true;

			if (const auto* toggle = m_registry.try_get<Toggle>(innerEntity))
			{
				active = toggle->active;
			}

			if (!active)
			{
				return;
			}

			if (innerHitbox.type == ColiderType::Hazard)
			{
				restart(ctx);
			}

			if (innerHitbox.type == ColiderType::Zone)
			{
				if (auto* trigger = m_registry.try_get<Trigger>(innerEntity))
				{
					if (api.mapping.isPressed("Interact"))
					{
						trigger->active = !trigger->active;
					}
				}
			}
		});

		if (m_registry.all_of<Controllable>(entity))
		{
			m_oldCameraPosition = m_cameraPosition;
			m_cameraPosition = px::lerp(m_cameraPosition, { transform.pos.x + transform.facing * 1.f, transform.pos.y - 1.0f }, 0.05f);
		}
	});

	auto particleView = m_registry.view<Transform, IsParticle>();

	particleView.each([&](auto& transform) {
		transform.oldPos = transform.pos;
		transform.pos += transform.vel * ctx.dt.asSeconds();
	});
}