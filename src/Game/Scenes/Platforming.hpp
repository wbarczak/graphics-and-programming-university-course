#pragma once

#include <entt/entt.hpp>

#include "pXL/pXL.hpp"

#include "Game/Context.hpp"
#include "Game/Tile.hpp"
#include "Game/Map.hpp"
#include "Game/Components.hpp"
#include "Game/Device.hpp"

namespace Scenes
{
	class Platforming : public px::Scene
	{
	public:

		Platforming(px::SceneInitCtx& ctx, Context& gctx);

		void update(px::UpdateCtx& ctx) override;
		void fixedUpdate(px::UpdateCtx& ctx) override;
		void draw(px::DrawCtx& ctx) const override;

	private:

		void animate(px::UpdateCtx& ctx);
		void computeLifetime(px::UpdateCtx ctx);
		void playerControlSystem(px::UpdateCtx& ctx);
		void movementAndColisionSystem(px::UpdateCtx& ctx);
		void updateAnimationStates(px::UpdateCtx& ctx);

		Context& m_ctx;

		entt::registry m_registry;

		sf::Time m_elapsed;

		Map m_map;

		std::vector<Device> m_devices;

		sf::Vector2f m_cameraPosition, m_oldCameraPosition;
	};
}