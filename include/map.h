#pragma once

#include "tile.h"
#include "vec.h"

typedef struct Map {
    Tilemap t;
    Vec2* tiles;
    Vec2 dimensions;
} Map;
