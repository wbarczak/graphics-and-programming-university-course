#pragma once

#include "pXL/pXL.hpp"

#include "Cell.hpp"

using TileMap = px::Grid<const Cell*>;

inline const TileMap TileMapEmpty({ 0, 0 }, nullptr);