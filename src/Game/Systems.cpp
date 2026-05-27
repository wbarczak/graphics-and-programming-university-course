#include "Systems.hpp"
#include "Components.hpp"

void Systems::allign(Level& level)
{
	auto view = level.entities.view<Stationary>();

	view.each(
		[](auto& stationary) {
			stationary.position.x = floorf(stationary.position.x) + stationary.alignment.x;
			stationary.position.y = floorf(stationary.position.y) + stationary.alignment.y;
		}
	);
}