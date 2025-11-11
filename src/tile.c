#include "tile.h"
#include "png_handler.h"
#include "vec.h"
#include <stdlib.h>

Vec2 tile_coords(Vec2 point, ssize_t tile_size, TileCorner corner) {
    switch (corner) {
    case NORTH_WEST:
        return vec2_mul(point, tile_size);
    case NORTH_EAST:
        return vec2_add(vec2_mul(point, tile_size), VEC2(tile_size - 1, 0));
    case SOUTH_WEST:
        return vec2_add(vec2_mul(point, tile_size), VEC2(0, tile_size - 1));
    case SOUTH_EAST:
        return vec2_add(vec2_mul(point, tile_size), tile_size - 1);
    }
}

void tilemap_deinit(Tilemap t) {
    if (t.pixels) {
        free(t.pixels);
        t.pixels = NULL;
    }
}

Tilemap tilemap_load_png(const char* png_path, Vec2 tile_dims, Vec2 tile_gaps) {
    Vec2 dimensions;
    RGBA* pixels = read_png_file(png_path, &dimensions);
    return (Tilemap){
        .pixels = pixels,
        .dimensions = dimensions,
        .dimensions_in_tiles =
            VEC2((size_t)((size_t)dimensions.x / (size_t)tile_dims.x),
                 (size_t)((size_t)dimensions.y / (size_t)tile_dims.y)),
        .num_tiles = (size_t)((size_t)dimensions.x / (size_t)tile_dims.x) *
                     (size_t)((size_t)dimensions.y / (size_t)tile_dims.y),

        .tile_dimensions = tile_dims,
        .tile_gaps = tile_gaps,
    };
}
