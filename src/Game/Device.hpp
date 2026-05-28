#pragma once

#include <vector>
#include <cstdint>

#include <SFML/System.hpp>
#include <entt/entt.hpp>

#include "Components.hpp"

enum class DeviceType : uint8_t
{
	And, Or, Timed
};

struct DeviceIO
{
	entt::entity entity;
	bool inverted{};
};

struct Device
{
	Device(DeviceType type = DeviceType::And) :
		type(type)
	{}

	std::vector<DeviceIO> in, out;
	sf::Time onTime, offTime, accumulated;
	DeviceType type;
};