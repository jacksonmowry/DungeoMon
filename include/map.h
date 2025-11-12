#pragma once

#include "tile.h"
#include "vec.h"
#include <stdbool.h>

#define TILE_NORMAL (0 << 0)
#define TILE_HORIZONTAL_FLIP (1 << 0)
#define TILE_VERTICAL_FLIP (1 << 1)
#define TILE_WALL (1 << 2)
#define TILE_ENEMY (1 << 3)
#define TILE_DOOR (1 << 4)
#define TILE_STAIRS (1 << 5)
#define TILE_LOOT (1 << 6)

#define TILE_NUM_ATTRIBUTES 8

#define TILE_NORMAL_STR "normal"
#define TILE_HORIZONTAL_FLIP_STR "horizontal flip"
#define TILE_VERTICAL_FLIP_STR "vertial flip"
#define TILE_WALL_STR "wall"
#define TILE_ENEMY_STR "enemy"
#define TILE_DOOR_STR "door"
#define TILE_STAIRS_STR "stairs"
#define TILE_LOOT_STR "loot"

typedef struct Map {
    Tilemap t;
    Vec2I* tiles;
    TileRotation* tile_rotations;
    uint16_t* tile_attributes;

    Vec2I dimensions;
} Map;

typedef enum SaveMode {
    TILE_NUM,
    TILE_NAME,
} SaveMode;

void map_deinit(Map m);

bool map_save(const Map m, const char* filename, SaveMode mode);

// TODO take a look into data ownership here
// We need to be given a tilemap because we store it directly for now
// This copies some of the data but not any of the important stuff that's
// allocated In the future we may way to just take this in by pointer
Map map_load(const char* filename, Tilemap t);

int map_tile_attributes_debug(const Map m, const Vec2I pos, char* buf,
                              size_t buf_len);
