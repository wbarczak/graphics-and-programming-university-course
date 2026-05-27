#pragma once

#include <unordered_map>
#include <set>

#include <SFML/Audio.hpp>

#include "pXL/pXL.hpp"

#include "Tile.hpp"
#include "EntityPrefab.hpp"

struct Context
{
	px::Registry<EntityPrefab> entities;
	std::unordered_map<std::string, Tile> tiles;
	std::unordered_map<std::string, sf::SoundBuffer> sounds;

	std::vector<std::string> maps = {
		"./resources/maps/1.json",
		"./resources/maps/2.json",
		"./resources/maps/3.json",
		"./resources/maps/4.json",
		"./resources/maps/5.json",
		"./resources/maps/6.json"
	};

	std::set<int32_t> levels{ 1 };
	int32_t selectedLevel{ 1 };
	int32_t unlockedLevel{ 1 };

	const std::unordered_map<char, std::pair<std::string, bool>> tempMapping{
		{' ', {"empty", false}},
		{'#', {"dirt", false}},
		{'%', {"cobble", false}},
		{'@', {"moss", false}},
		{'w', {"spike_up", true}},
		{'a', {"spike_left", true}},
		{'s', {"spike_down", true}},
		{'d', {"spike_right", true}},
		{'i', {"tspike_up", true}},
		{'j', {"tspike_left", true}},
		{'k', {"tspike_down", true}},
		{'l', {"tspike_right", true}},
		{'c', {"cloud", true}},
		{'p', {"platform", true}},
		{'m', {"fungi", true}},
		{'b', {"button", true}},
		{'t', {"toggle_block", true}}
	};
};