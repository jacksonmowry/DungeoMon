#include "tile.h"
#include "png_handler.h"
#include "vec.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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

    if (t.tile_names) {
        for (size_t i = 0; i < t.num_tiles; i++) {
            free(t.tile_names[i]);
        }
        free(t.tile_names);
        t.tile_names = NULL;
    }
}

Tilemap tilemap_load(const char* pam_path, const char* tile_names_path,
                     Vec2 tile_dims, Vec2 tile_gaps) {
    Vec2 dimensions;
    RGBA* pixels = read_pam_file(pam_path, &dimensions);
    char** tile_names = NULL;
    if (tile_names_path) {
        FILE* fp = fopen(tile_names_path, "r");
        if (!fp) {
            goto SKIP;
        }
        tile_names = malloc(sizeof(*tile_names) * (dimensions.x / tile_dims.x) *
                            (dimensions.y / tile_dims.y));
        for (size_t i = 0;
             i < (dimensions.x / tile_dims.x) * (dimensions.y / tile_dims.y);
             i++) {
            char buf[256];
            if (fscanf(fp, "%s", buf) != 1) {
                fprintf(stderr, "Error after reading %zu/%.0f tile names\n", i,
                        dimensions.x * dimensions.y);
                exit(1);
            }

            tile_names[i] = strdup(buf);
        }
    }

SKIP:
    return (Tilemap){
        .pixels = pixels,
        .dimensions = dimensions,
        .dimensions_in_tiles =
            VEC2((size_t)((size_t)dimensions.x / (size_t)tile_dims.x),
                 (size_t)((size_t)dimensions.y / (size_t)tile_dims.y)),
        .num_tiles = (size_t)((size_t)dimensions.x / (size_t)tile_dims.x) *
                     (size_t)((size_t)dimensions.y / (size_t)tile_dims.y),

        .tile_names = tile_names,

        .tile_dimensions = tile_dims,
        .tile_gaps = tile_gaps,
    };
}
