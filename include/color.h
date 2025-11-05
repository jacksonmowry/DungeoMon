#pragma once

#include <stdint.h>

typedef struct RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

typedef struct RGBA {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} RGBA;

#define RGBA(rgb) ((RGBA){.r = rgb.r, .g = rgb.g, .b = rgb.b, .a = 0xFF})

// Modify a colors brightness, b is in [0,1]
#define RGB_BRIGHTNESS(c, br)                                                  \
    ((RGBA){                                                                   \
        .r = (c.r * (br)), .g = (c.g * (br)), .b = (c.b * (br)), .a = (c.a)})

#define WHITE ((RGB){.r = 0xFF, .g = 0xFF, .b = 0xFF})
#define BLACK ((RGB){.r = 0x00, .g = 0x00, .b = 0x00})
#define RED ((RGB){.r = 0xFF, .g = 0x00, .b = 0x00})
#define BLUE ((RGB){.r = 0x00, .g = 0x00, .b = 0xFF})
#define GREEN ((RGB){.r = 0x00, .g = 0xFF, .b = 0x00})

RGB color_blend(RGB base_color, RGBA new_color);
