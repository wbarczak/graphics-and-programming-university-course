#pragma once

#include <entt/entt.hpp>

#include <nlohmann/json.hpp>

#include "Map.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace nl = nlohmann;


struct World
{
	entt::registry entities;
	Map map;
	World() : map(sf::Vector2u(25, 25), Tile{}) {}
};


void saveWorld( const std::filesystem::path& path, const World& world);


void loadWorld(	const std::filesystem::path& path,	
	const std::unordered_map<std::string, Tile>& tiles,
	World& world);