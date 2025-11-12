#pragma once

#include "color.h"
#include "vec.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

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

// clock wise
typedef enum TileRotation {
    _0,
    _90,
    _180,
    _270,
} TileRotation;

// 0,0 is the top left of the screen
// w-1,h-1 is the bottom right
// Give the upper left coordinate of the provided tile
Vec2 tile_coords(Vec2 point, ssize_t tile_size, TileCorner corner);

typedef struct Tilemap {
    RGBA* pixels;
    Vec2 dimensions;
    Vec2 dimensions_in_tiles;
    size_t num_tiles;

    Vec2 tile_dimensions;
    Vec2 tile_gaps;
} Tilemap;

void tilemap_deinit(Tilemap t);
Tilemap tilemap_load_png(const char* png_path, Vec2 tile_dims, Vec2 tile_gaps);
