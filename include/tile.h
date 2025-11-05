#pragma once

#include "vec.h"
#include <stddef.h>

typedef enum TileCorner {
    NORTH_WEST = 0,
    NORTH_EAST = 1,
    SOUTH_WEST = 2,
    SOUTH_EAST = 3,
    NW = 0,
    NE = 1,
    SW = 2,
    SE = 3
} TileCorner;

// 0,0 is the top left of the screen
// w-1,h-1 is the bottom right
// Give the upper left coordinate of the provided tile
Vec2 tile_coords(Vec2 point, size_t tile_size, TileCorner corner);
