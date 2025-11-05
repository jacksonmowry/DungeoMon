#pragma once

#include "color.h"
#include "tile.h"
#include "vec.h"
#include <stddef.h>
#include <stdint.h>

typedef struct Renderer {
    void* state;

    size_t height;
    size_t width;

    size_t tile_size;

    void (*draw_line)(void* state, Vec2 p1, Vec2 p2, RGBA color);
    void (*draw_rect)(void* state, Vec2 p1, Vec2 p2, RGBA color);
    void (*draw_rect_filled)(void* state, Vec2 p1, Vec2 p2, RGBA border_color,
                             RGBA fill_color);
    void (*draw_pixel)(void* state, Vec2 p1, RGBA color);
    void (*draw_text)(void* state, char* str, Vec2 pos, RGBA color,
                      size_t scale);
    void (*draw_tile)(void* state, Vec2 dest_tile, Vec2 tile_shift, Tilemap t,
                      Vec2 source_tile, TileRotation rot, int flip);
    void (*render)(void* state);
    void (*cleanup)(void* state);
} Renderer;
