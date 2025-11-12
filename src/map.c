#include "map.h"
#include <stdbool.h>

int map_tile_attributes_debug(const Map m, const Vec2 pos, char* buf,
                              size_t buf_len) {
    const char* t = "true";
    const char* f = "false";

    size_t index = (pos.y * m.dimensions.x) + pos.x;
    uint16_t attributes = m.tile_attributes[index];

    const char* normal = attributes & TILE_NORMAL ? t : f;
    const char* horizontal_flip = attributes & TILE_HORIZONTAL_FLIP ? t : f;
    const char* vertical_flip = attributes & TILE_VERTICAL_FLIP ? t : f;
    const char* wall = attributes & TILE_WALL ? t : f;
    const char* enemy = attributes & TILE_ENEMY ? t : f;
    const char* door = attributes & TILE_DOOR ? t : f;
    const char* stairs = attributes & TILE_STAIRS ? t : f;
    const char* loot = attributes & TILE_LOOT ? t : f;
    char* rotation;
    switch (m.tile_rotations[index]) {
    case _0:
        rotation = "0";
        break;
    case _90:
        rotation = "90";
        break;
    case _180:
        rotation = "180";
        break;
    case _270:
        rotation = "270";
        break;
    }

    Vec2 actual_tile =
        m.tiles[(size_t)(pos.y * m.dimensions.x) + (size_t)pos.x];

    return snprintf(
        buf, buf_len - 1,
        "Map Tile (%.0f, %.0f), Tile Name %s: {\r\n\t\"normal\": "
        "%s,\r\n\t\"horizontal_flip\": "
        "%s,\r\n\t\"vertical_flip\": %s,\r\n\t\"rotation\": %s,\r\n\t\"wall\": "
        "%s,\r\n\t\"enemy\": "
        "%s,\r\n\t\"door\": %s,\r\n\t\"stairs\": %s,\r\n\t\"loot\": %s\r\n}",
        pos.x, pos.y,
        m.t.tile_names[(size_t)(actual_tile.y * m.t.dimensions_in_tiles.x) +
                       (size_t)actual_tile.x],
        normal, horizontal_flip, vertical_flip, rotation, wall, enemy, door,
        stairs, loot);
}
