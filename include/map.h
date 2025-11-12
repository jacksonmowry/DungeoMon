#pragma once

#include "tile.h"
#include "vec.h"

#define TILE_NORMAL 0 << 0
#define TILE_WALL 1 << 0
#define TILE_ENEMY 1 << 1
#define TILE_DOOR 1 << 2
#define TILE_STAIRS 1 << 3
#define TILE_LOOT 1 << 4

typedef struct Map {
    Tilemap t;
    Vec2* tiles;
    uint16_t* tile_attributes;

    Vec2 dimensions;
} Map;
