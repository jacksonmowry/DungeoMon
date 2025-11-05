#pragma once

#include "vec.h"
#include <stddef.h>
#include <stdint.h>

typedef struct RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

// Modify a colors brightness, b is in [0,1]
#define RGB_BRIGHTNESS(c, br)                                                  \
    ((RGB){.r = (c.r * (br)), .g = (c.g * (br)), .b = (c.b * (br))})

#define WHITE ((RGB){.r = 0xFF, .g = 0xFF, .b = 0xFF})
#define BLACK ((RGB){.r = 0x00, .g = 0x00, .b = 0x00})
#define RED ((RGB){.r = 0xFF, .g = 0x00, .b = 0x00})
#define BLUE ((RGB){.r = 0x00, .g = 0x00, .b = 0xFF})
#define GREEN ((RGB){.r = 0x00, .g = 0xFF, .b = 0x00})

typedef struct Renderer {
    void* state;

    size_t height;
    size_t width;

    size_t tile_size;

    void (*draw_line)(void* state, Vec2 p1, Vec2 p2, RGB color);
    void (*draw_rect)(void* state, Vec2 p1, Vec2 p2, RGB color);
    void (*draw_rect_filled)(void* state, Vec2 p1, Vec2 p2, RGB border_color,
                             RGB fill_color);
    void (*draw_text)(void* state, char* str, Vec2 pos, RGB color,
                      size_t scale);
    void (*render)(void* state);
    void (*cleanup)(void* state);
} Renderer;
