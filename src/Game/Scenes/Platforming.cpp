#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <optional>

#include "Platforming.hpp"
#include "Game/Constants.hpp"
#include "Game/Device.hpp"

Scenes::Platforming::Platforming(px::SceneInitCtx& ctx, Context& gctx) :
	Scene(ctx),
	m_ctx(gctx),
	m_map(sf::Vector2u(40, 20), m_ctx.tiles["empty"])
{
	const char* mapBuilder[]{
		"########################################",
		"#                                      #",
		"#                                ##    #",
		"#   ######          #####              #",
		"#                                      #",
		"#          ###               ##        #",
		"#                                      #",
		"#                       ##             #",
		"#### ##############                    #",
		"#                                      #",
		"#  #                  ####             #",
		"#     ##    #                          #",
		"#                           #          #",
		"#                                      #",
		"#    ##   ###                  #       #",
		"#                    #                 #",
		"##               #         #           #",
		"#                                      #",
		"#                                      #",
		"########################################",
	};

	for (uint32_t y = 0; y < m_map.size().y; ++y)
	{
		for (uint32_t x = 0; x < m_map.size().x; ++x)
		{
			if (mapBuilder[y][x] == '#')
			{
				m_map.at({ x, y }) = m_ctx.tiles.at("solid_block");
			}
		}
	}

	{
		auto player = m_ctx.entities.get("player").spawn(m_registry);
		auto& transform = m_registry.get<Transform>(player);
		transform.pos = { 2.5f, 2.5f };
		transform.oldPos = transform.pos;
	}

	{
		auto spike = m_ctx.entities.get("spike_up").spawn(m_registry);
		auto& stationary = m_registry.get<Stationary>(spike);
		stationary.position = {8.f, 7.f};
	}

	{
		auto spike = m_ctx.entities.get("spike_down").spawn(m_registry);
		auto& stationary = m_registry.get<Stationary>(spike);
		stationary.position = { 8.f, 1.f };
	}

	{
		auto spike = m_ctx.entities.get("spike_right").spawn(m_registry);
		auto& stationary = m_registry.get<Stationary>(spike);
		stationary.position = { 14.f, 5.f };
	}

	{
		auto platform = m_ctx.entities.get("platform").spawn(m_registry);
		auto& stationary = m_registry.get<Stationary>(platform);
		stationary.position = { 6.f, 6.f };
	}

	auto retractableSpike = m_ctx.entities.get("retractable_spike").spawn(m_registry);

	{
		auto& stationary = m_registry.get<Stationary>(retractableSpike);
		stationary.position = { 7.f, 7.f };
	}

	{
		Device device;
		device.type = DeviceType::Timed;
		device.onTime = sf::seconds(5);
		device.offTime = sf::seconds(5);
		device.out.push_back({ retractableSpike });
		m_devices.push_back(std::move(device));
	}
}

void Scenes::Platforming::update(px::UpdateCtx& ctx)
{
	m_elapsed += ctx.dt;

	if (api.mapping.isPressed("Pause"))
	{
		api.comms.push("Pause");
	}

	animate(ctx);
}

void Scenes::Platforming::fixedUpdate(px::UpdateCtx& ctx)
{
	computeLifetime(ctx);

	for (auto& device : m_devices)
	{
		switch (device.type)
		{
		case DeviceType::Timed:
			
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
		position.y = std::min(position.y, m_map.size().y - halfScreenTiles.y);
		position.x = std::max(position.x, halfScreenTiles.x);
		position.y = std::max(position.y, halfScreenTiles.y);

		ctx.window.draw(px::Background(api.assets.backgrounds.get("background"), position.x * unitPixels));

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

	auto view = m_registry.view<const px::Animation, const Transform>();

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

	auto hitboxView = m_registry.view<const Hitbox>();

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
	});
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
	toggleView.each([&](const auto& toggle, auto& animation) {
		if (toggle.active)
		{
			animation.play("active");
		}
		else
		{
			animation.play("inactive");
		}
	});
}

void Scenes::Platforming::computeLifetime(px::UpdateCtx ctx)
{
	auto view = m_registry.view<Lifetime>();

	view.each([&](entt::entity entity, auto& lifetime) {
		lifetime.lived += ctx.dt;
		if (lifetime.lived > lifetime.max)
		{
			m_registry.destroy(entity);
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
			controllable.jumpBuffer = sf::seconds(999);
			controllable.jumpBuffer += ctx.dt;
		}

		controllable.cayoteTime += ctx.dt;

		sf::Vector2 bottomLeft = transform.pos + hitbox.rect.position;
		bottomLeft.y += hitbox.rect.size.y;
		sf::Vector2f bottomRight = bottomLeft;
		bottomRight.x += hitbox.rect.size.x;

		if (!controllable.wasGrounded && controllable.grounded && transform.pos.y > transform.jumpStartY - 1e-3)
		{
			for (size_t i = 0; i < 10; ++i)
			{
				auto particle = m_ctx.entities.get("cloud_particle").spawn(m_registry);
				auto& particleTransform = m_registry.get<Transform>(particle);

				particleTransform.pos = transform.pos;
				particleTransform.oldPos = transform.pos;

				float angle = static_cast<float>(rand()) / RAND_MAX * 2.f * M_PI;
				sf::Vector2f direction(cosf(angle), sinf(angle));

				particleTransform.vel = direction * 1.5f;
			}
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

			ColiderType type = colider.entity ? m_registry.get<Hitbox>(colider.entity.value()).type : ColiderType::Solid;
			auto* toggle = colider.entity ? m_registry.try_get<Toggle>(colider.entity.value()) : nullptr;

			bool goodAngle = (type == ColiderType::Platform && result.normal == sf::Vector2f{ 0.f,-1.f }) || type != ColiderType::Platform;
			bool active = !toggle || toggle->active;
			bool resolve = goodAngle && active;

			if (resolve)
			{
				transform.vel += sf::Vector2f{
					result.normal.x * std::abs(transform.vel.x),
					result.normal.y * std::abs(transform.vel.y)
				} *(1.0f - result.time);

				deltaDistance = transform.vel * ctx.dt.asSeconds();
			}

			if (type == ColiderType::Hazard && resolve && !ctx.transition.isActive())
			{
				ctx.transition.start([&]()
				{
					api.comms.replace("Platforming");
				});
			}

			if (auto* controllable = m_registry.try_get<Controllable>(entity))
			{
				if (result.normal.y < 0.f)
				{
					controllable->grounded = true;
				}
			}
		}

		transform.pos += transform.vel * ctx.dt.asSeconds();

		stationaryView.each([&](entt::entity innerEntity, const Stationary& innerStationary, const Hitbox& innerHitbox)
		{
			auto colider = innerHitbox.rect;
			colider.position += innerStationary.position;

			bool aabb = colider.position.x <= entityRect.position.x + entityRect.size.x
				&& colider.position.y <= entityRect.position.y + entityRect.size.y
				&& colider.position.x + colider.size.x >= entityRect.position.x
				&& colider.position.y + colider.size.y >= entityRect.position.y;

			if (!aabb)
			{
				return;
			}

			bool active = true;

			if (const auto* toggle = m_registry.try_get<Toggle>(innerEntity))
			{
				active = toggle->active;
			}

			if (active && innerHitbox.type == ColiderType::Hazard && !ctx.transition.isActive())
			{
				ctx.transition.start([&]()
				{
					api.comms.replace("Platforming");
				});
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