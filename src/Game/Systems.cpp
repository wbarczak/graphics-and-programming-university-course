#include "Systems.hpp"
#include "Components.hpp"

void Systems::allign(entt::registry& level)
{
	auto view = level.view<Stationary>();

	view.each(
		[](auto& stationary) {
			stationary.position.x = floorf(stationary.position.x) + stationary.alignment.x;
			stationary.position.y = floorf(stationary.position.y) + stationary.alignment.y;
		}
	);
}