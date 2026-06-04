#pragma once

#include "World.hpp"

#include <entt/entt.hpp>

#include <nlohmann/json.hpp>

#include "Map.hpp"

#include <fstream>
#include <iostream>
#include <filesystem>

#include "Components.hpp"

namespace nl = nlohmann;

void saveWorld(
	const std::filesystem::path& path,
	const World& world)
{
	(std::filesystem::exists(path))
		? SPDLOG_INFO("Saved the map to existing file: {}", path.string())
		: SPDLOG_INFO("Created new save file: {}", path.string());

	const auto& worldMap = world.map;
	const auto& worldEntities = world.entities;

	nl::json mapSaveJ;
	mapSaveJ["width"] = worldMap.size().x;
	mapSaveJ["height"] = worldMap.size().y;

	nl::json rows = nl::json::array();
	nl::json saveEntities = nl::json::array();

	for (uint32_t y = 0; y < worldMap.size().y; y++) {
		nl::json row = nl::json::array();
		for (uint32_t x = 0; x < worldMap.size().x; x++) {
			row.push_back(worldMap.at({ x,y }).tileName);
		}
		rows.push_back(row);
	}
	mapSaveJ["tiles"] = rows;

	worldEntities.view<const Transform, const EntityType>().each(
		[&](const Transform& t, const EntityType& et) {
			nl::json ent;
			ent["type"] = et.name;
			ent["x"] = t.pos.x;
			ent["y"] = t.pos.y;
			saveEntities.push_back(ent);
		}
	);
	mapSaveJ["entities"] = saveEntities;
	std::ofstream mapSave(path);
	mapSave << mapSaveJ.dump(4);
	mapSave.close();
}

void loadWorld(
	const std::filesystem::path& path,
	const std::unordered_map<std::string, Tile>& tiles,
	World& world)
{
	std::ifstream loadedMap(path);
	if (!loadedMap.is_open()) {
		SPDLOG_ERROR("Failed to load the map from {}", path.string());
		return;
	}

	nl::json loadedMapJson = nl::json::parse(loadedMap);
	uint32_t width = loadedMapJson["width"];
	uint32_t height = loadedMapJson["height"];

	sf::Vector2u size = { width, height };
	world.map = Map(size, tiles.at("empty"));

	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			std::string loadedTileName = loadedMapJson["tiles"][y][x];
			if (tiles.contains(loadedTileName)) {
				world.map.at({ x,y }) = tiles.at(loadedTileName);
			}
			else {
				SPDLOG_INFO("Given '{}' tile doesnt exist", loadedTileName);
			}
		}
	}


	SPDLOG_INFO("Loaded the map from {}", path.string());

}
