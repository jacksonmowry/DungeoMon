#include "tile.h"
#include "png_handler.h"
#include "vec.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

Vec2I tile_coords(Vec2I point, ssize_t tile_size, TileCorner corner) {
    switch (corner) {
    case NORTH_WEST:
        return vec2i_mul(point, tile_size);
    case NORTH_EAST:
        return vec2i_add(vec2i_mul(point, tile_size), VEC2I(tile_size - 1, 0));
    case SOUTH_WEST:
        return vec2i_add(vec2i_mul(point, tile_size), VEC2I(0, tile_size - 1));
    case SOUTH_EAST:
        return vec2i_add(vec2i_mul(point, tile_size), tile_size - 1);
    }

    return (Vec2I){0};
}

void tilemap_deinit(Tilemap t) {
    if (t.id) {
        free((void*)t.id);
    }

    if (t.pixels) {
        free((void*)t.pixels);
        t.pixels = NULL;
    }

    if (t.tile_names) {
        for (size_t i = 0; i < t.num_tiles; i++) {
            free((void*)t.tile_names[i]);
        }
        free(t.tile_names);
        t.tile_names = NULL;
    }
}

Tilemap tilemap_load(const char* pam_path, const char* tile_names_path,
                     Vec2I tile_dims, Vec2I tile_gaps) {
    Vec2I dimensions;
    const RGBA* pixels = read_pam_file(pam_path, &dimensions);
    const char** tile_names = NULL;
    if (tile_names_path) {
        FILE* fp = fopen(tile_names_path, "r");
        if (!fp) {
            goto SKIP;
        }
        tile_names = malloc(sizeof(*tile_names) * (dimensions.x / tile_dims.x) *
                            (dimensions.y / tile_dims.y));
        for (size_t i = 0; i < (size_t)((dimensions.x / tile_dims.x) *
                                        (dimensions.y / tile_dims.y));
             i++) {
            char buf[256];
            if (fscanf(fp, "%s", buf) != 1) {
                fprintf(stderr, "Error after reading %zu/%d tile names\n", i,
                        dimensions.x * dimensions.y);
                exit(1);
            }

            tile_names[i] = strdup(buf);
        }
        fclose(fp);
    }

SKIP:;
    char* name = realpath(pam_path, NULL);
    char* last_slash = strrchr(name, '/');
    last_slash++;
    *strrchr(last_slash, '.') = '\0';
    const char* tilemap_id = strdup(last_slash);

    free(name);

    return (Tilemap){
        .id = tilemap_id,

        .pixels = pixels,
        .dimensions = dimensions,
        .dimensions_in_tiles =
            VEC2I((dimensions.x / tile_dims.x), (dimensions.y / tile_dims.y)),
        .num_tiles =
            (dimensions.x / tile_dims.x) * (dimensions.y / tile_dims.y),

        .tile_names = tile_names,

        .tile_dimensions = tile_dims,
        .tile_gaps = tile_gaps,
    };
}
