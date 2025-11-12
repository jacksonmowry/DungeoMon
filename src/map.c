#include "map.h"
#include "vec.h"
#include <assert.h>
#include <ctype.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void map_deinit(Map m) {
    if (m.tiles) {
        free(m.tiles);
        m.tiles = NULL;
    }

    if (m.tile_rotations) {
        free(m.tile_rotations);
        m.tile_rotations = NULL;
    }

    if (m.tile_attributes) {
        free(m.tile_attributes);
        m.tile_attributes = NULL;
    }
}

bool map_save(const Map m, const char* filename, SaveMode mode) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror(filename);
        exit(1);
    }

    fprintf(fp, "WIDTH      %d\n", (int)m.dimensions.x);
    fprintf(fp, "HEIGHT     %d\n", (int)m.dimensions.y);
    fprintf(fp, "TILEMAP ID %s\n", m.t.id);
    fprintf(fp, "MODE       %s\n",
            (mode == TILE_NUM) ? "tile_num" : "tile_name");
    fprintf(fp, "\n");

    fprintf(fp, "MAP TILES\n");
    for (size_t row = 0; row < m.dimensions.y; row++) {
        for (size_t col = 0; col < m.dimensions.x; col++) {
            size_t map_index = (row * m.dimensions.x) + col;
            Vec2I tile = m.tiles[map_index];
            size_t tile_index = (tile.y * m.t.dimensions_in_tiles.x) + tile.x;

            if (mode == TILE_NUM) {
                fprintf(fp, "%3zu", tile_index);
            } else {
                fprintf(fp, "%s", m.t.tile_names[tile_index]);
            }

            if (col != m.dimensions.x - 1) {
                fprintf(fp, " ");
            } else {
                fprintf(fp, "\n");
            }
        }
    }
    fprintf(fp, "\n");

    fprintf(fp, "MAP TILE ATTRIBUTES\n");
    for (size_t row = 0; row < m.dimensions.y; row++) {
        for (size_t col = 0; col < m.dimensions.x; col++) {
            size_t map_index = (row * m.dimensions.x) + col;
            uint16_t tile_attributes = m.tile_attributes[map_index];

            fprintf(fp, "%04hX", tile_attributes);

            if (col != m.dimensions.x - 1) {
                fprintf(fp, " ");
            } else {
                fprintf(fp, "\n");
            }
        }
    }
    fprintf(fp, "\n");

    fprintf(fp, "MAP TILE ROTATIONS\n");
    for (size_t row = 0; row < m.dimensions.y; row++) {
        for (size_t col = 0; col < m.dimensions.x; col++) {
            size_t map_index = (row * m.dimensions.x) + col;
            int tile_rotations = m.tile_rotations[map_index];

            fprintf(fp, "%1d", tile_rotations);

            if (col != m.dimensions.x - 1) {
                fprintf(fp, " ");
            } else {
                fprintf(fp, "\n");
            }
        }
    }
    fprintf(fp, "\n");

    fclose(fp);

    return true;
}

Map map_load(const char* filename, Tilemap t) {
    Map m = {0};

    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror(filename);
        exit(1);
    }

    char buf[256];

    // WIDTH x
    fgets(buf, sizeof(buf), fp);
    if (sscanf(buf, "WIDTH %d", &m.dimensions.x) != 1) {
        fprintf(stderr, "Error while reading map, expected: WIDTH x, got: %s\n",
                buf);
        exit(1);
    }
    // HEIGHT y
    fgets(buf, sizeof(buf), fp);
    if (sscanf(buf, "HEIGHT %d", &m.dimensions.y) != 1) {
        fprintf(stderr,
                "Error while reading map, expected: HEIGHT y, got: %s\n", buf);
        exit(1);
    }
    // TILEMAP ID id
    fgets(buf, sizeof(buf), fp);
    char id[512];
    if (sscanf(buf, "TILEMAP ID %s", id) != 1) {
        fprintf(stderr,
                "Error while reading map, expected: TILEMAP ID id, got: %s\n",
                buf);
        exit(1);
    }
    if (strcmp(id, t.id)) {
        fprintf(stderr,
                "Error while reading map, expected tilemap id: %s, got: %s\n",
                t.id, id);
    }
    // MODE mode
    fgets(buf, sizeof(buf), fp);
    char mode_buf[256] = {0};
    if (sscanf(buf, "MODE %s", mode_buf) != 1) {
        fprintf(stderr,
                "Error while reading map, expected: MODE mode, got: %s\n", buf);
        exit(1);
    }
    SaveMode sm = (!strncmp("tile_num", mode_buf, 8)) ? TILE_NUM : TILE_NAME;
    // MAP TILES
    do {
        fgets(buf, sizeof(buf), fp);
    } while (strlen(buf) == 0 || isspace(buf[0]));
    if (strncmp("MAP TILES", buf, 9)) {
        fprintf(stderr,
                "Error while reading map, expected: MAP TILES\\n, got: %s\n",
                buf);
        exit(1);
    }

    // Finally allocate the buffer and begin reading tiles
    m.tiles = malloc(m.dimensions.x * m.dimensions.y * sizeof(*m.tiles));
    for (size_t row = 0; row < m.dimensions.y; row++) {
        for (size_t col = 0; col < m.dimensions.x; col++) {
            size_t map_index = (row * m.dimensions.x) + col;
            Vec2I tile_pos;

            if (sm == TILE_NUM) {
                size_t tile_index;
                if (fscanf(fp, "%zu", &tile_index) != 1) {
                    fprintf(stderr, "Error reading tile %zu/%d\n", map_index,
                            m.dimensions.x * m.dimensions.y);
                    exit(1);
                }

                tile_pos = VEC2I(tile_index % t.dimensions_in_tiles.x,
                                 (tile_index / t.dimensions_in_tiles.x));
            } else {
                assert(false);
            }

            m.tiles[map_index] = tile_pos;
        }
    }

    // MAP TILE ATTRIBUTES
    do {
        fgets(buf, sizeof(buf), fp);
    } while (strlen(buf) == 0 || isspace(buf[0]));
    if (strncmp("MAP TILE ATTRIBUTES", buf, 19)) {
        fprintf(stderr,
                "Error while reading map, expected: MAP TILE ATTRIBUTES\\n, "
                "got: %s\n",
                buf);
        exit(1);
    }
    // Attributes
    m.tile_attributes =
        malloc(m.dimensions.x * m.dimensions.y * sizeof(*m.tile_attributes));
    for (size_t row = 0; row < m.dimensions.y; row++) {
        for (size_t col = 0; col < m.dimensions.x; col++) {
            size_t map_index = (row * m.dimensions.x) + col;
            if (fscanf(fp, "%hX", m.tile_attributes + map_index) != 1) {
                fprintf(stderr, "Error reading tile attribute %zu/%d\n",
                        map_index, m.dimensions.x * m.dimensions.y);
                exit(1);
            }
        }
    }

    // MAP TILE ROTATIONS
    do {
        fgets(buf, sizeof(buf), fp);
    } while (strlen(buf) == 0 || isspace(buf[0]));
    if (strncmp("MAP TILE ROTATIONS", buf, 18)) {
        fprintf(stderr,
                "Error while reading map, expected: MAP TILE ROTATIONS\\n, "
                "got: %s\n",
                buf);
        exit(1);
    }
    // Rotations
    m.tile_rotations =
        malloc(m.dimensions.x * m.dimensions.y * sizeof(*m.tile_rotations));
    for (size_t row = 0; row < m.dimensions.y; row++) {
        for (size_t col = 0; col < m.dimensions.x; col++) {
            size_t map_index = (row * m.dimensions.x) + col;
            if (fscanf(fp, "%d", m.tile_rotations + map_index) != 1) {
                fprintf(stderr, "Error reading tile rotation %zu/%d\n",
                        map_index, m.dimensions.x * m.dimensions.y);
                exit(1);
            }
        }
    }

    m.t = t;

    return m;
}

int map_tile_attributes_debug(const Map m, const Vec2I pos, char* buf,
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

    Vec2I actual_tile =
        m.tiles[(size_t)(pos.y * m.dimensions.x) + (size_t)pos.x];

    return snprintf(
        buf, buf_len - 1,
        "Map Tile (%d, %d), Tile Name %s: {\r\n\t\"normal\": "
        "%s,\r\n\t\"horizontal_flip\": "
        "%s,\r\n\t\"vertical_flip\": %s,\r\n\t\"rotation\": %s,\r\n\t\"wall\": "
        "%s,\r\n\t\"enemy\": "
        "%s,\r\n\t\"door\": %s,\r\n\t\"stairs\": %s,\r\n\t\"loot\": %s\r\n}",
        pos.x, pos.y,
        m.t.tile_names ? m.t.tile_names[(size_t)(actual_tile.y *
                                                 m.t.dimensions_in_tiles.x) +
                                        (size_t)actual_tile.x]
                       : "n/a",
        normal, horizontal_flip, vertical_flip, rotation, wall, enemy, door,
        stairs, loot);
}
