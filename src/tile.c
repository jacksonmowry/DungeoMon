#include "tile.h"

Vec2 tile_coords(Vec2 point, size_t tile_size, TileCorner corner) {
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
