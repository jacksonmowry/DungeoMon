#include "renderer.h"
#include "sx.h"
#include <sixel.h>
#include <stdint.h>

typedef struct SixelState {
    uint8_t* pixels;

    sixel_dither_t* dither;
    sixel_output_t* context;

    size_t height;
    size_t width;
} SixelState;

void draw_line(void* state, size_t x1, size_t y1, size_t x2, size_t y2);
void render(void* state, double min, double max, size_t x_max);
void cleanup(void* state);

Renderer sx_init(size_t width, size_t height);
