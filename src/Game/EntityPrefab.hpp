#pragma once

#include "pXL/pXL.hpp"

#include "Components.hpp"

using EntityPrefab = px::EntityPrefab<
	Hitbox,
	Transform,
	Stationary,
	Controllable,
	Lifetime,
	IsParticle,
	px::Animation,
	ColiderType,
	Trigger,
	Toggle
>;