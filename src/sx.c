#include "sx.h"
#include "font.h"
#include "renderer.h"
#include "vec.h"
#include <assert.h>
#include <math.h>
#include <sixel.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define max(x, y) (x > y ? x : y)
#define min(x, y) (x < y ? x : y)

typedef struct SixelState {
    RGB* pixels;
    RGB* upscale;

    sixel_dither_t* dither;
    sixel_output_t* context;

    size_t height;
    size_t width;
    size_t scale;
} SixelState;

static void set_pixel(SixelState* s, size_t x, size_t y) {
    if (x >= s->width || y >= s->height) {
        return;
    }
    s->pixels[(y * s->width) + x] = WHITE;
}

static void set_pixel_color(SixelState* s, size_t x, size_t y, RGB color) {
    if (x >= s->width || y >= s->height) {
        return;
    }
    s->pixels[(y * s->width) + x] = color;
}

static void fill_region(SixelState* s, Vec2 p1, Vec2 p2, RGB color) {
    size_t x1 = p1.x;
    size_t y1 = p1.y;
    size_t x2 = p2.x;
    size_t y2 = p2.y;

    for (size_t row = min(y1, y2); row < max(y1, y2); row++) {
        for (size_t col = min(x1, x2); col < max(x1, x2); col++) {
            set_pixel_color(s, col, row, color);
        }
    }
}

// Draws from top left
static void draw_char(SixelState* s, char c, size_t row, size_t col, RGB color,
                      size_t scale) {
    size_t x = col;
    size_t y = row;

    if (x >= s->width || y >= s->height) {
        return;
    }

    for (size_t i = 0; i < FONT_DIM * scale; i++) {
        size_t internal_i = i / scale;
        uint8_t row = font8x8_basic[c][internal_i];
        for (size_t j = 0; j < FONT_DIM * scale; j++) {
            size_t internal_j = j / scale;
            if ((row >> internal_j) & 1) {
                set_pixel_color(s, x + j, y + i, color);
            }
        }
    }
}

static void draw_string(void* state, char* str, Vec2 pos, RGB color,
                        size_t scale) {
    SixelState* s = (SixelState*)state;

    for (size_t i = 0; i < strlen(str); i++) {
        draw_char(s, str[i], pos.y, pos.x + (i * FONT_DIM * scale), color,
                  scale);
    }
}

static int sixel_write(char* data, int size, void* priv) {
    FILE* f = (FILE*)priv;
    return write(f->_fileno, data, size);
}

static double slope(double x1, double y1, double x2, double y2) {
    double bottom = x2 - x1;
    if (bottom == 0) {
        return 0;
    }

    return (y2 - y1) / (x2 - x1);
}

void draw_hline(void* state, size_t x1, size_t y1, size_t x2, RGB color) {
    SixelState* s = (SixelState*)state;
    for (size_t col = min(x1, x2); col <= max(x1, x2); col++) {
        set_pixel_color(s, col, y1, color);
    }
}
void draw_vline(void* state, size_t x1, size_t y1, size_t y2, RGB color) {
    SixelState* s = (SixelState*)state;
    for (size_t row = min(y1, y2); row < max(y1, y2); row++) {
        set_pixel_color(s, x1, row, color);
    }
}

void draw_line(void* state, Vec2 p1, Vec2 p2, RGB color) {
    SixelState* s = (SixelState*)state;

    size_t x1 = p1.x;
    size_t y1 = p1.y;
    size_t x2 = p2.x;
    size_t y2 = p2.y;

    if (x1 == x2) {
        return draw_vline(state, x1, y1, y2, color);
    } else if (y1 == y2) {
        return draw_hline(state, x1, y1, x2, color);
    }

    double x1f = y1;
    double y1f = x1;
    double x2f = y2;
    double y2f = x2;
    bool steep = fabs(y2f - y1f) > fabs(x2f - x1f);

    if (steep) {
        double tmp = x1f;
        x1f = y1f;
        y1f = tmp;

        tmp = x2f;
        x2f = y2f;
        y2f = tmp;
    }

    if (x1f > x2f) {
        double tmp = x1f;
        x1f = x2f;
        x2f = tmp;

        tmp = y1f;
        y1f = y2f;
        y2f = tmp;
    }

    const double dx = x2f - x1f;
    const double dy = y2f - y1f;
    const double gradient = dx == 0 ? 1 : dy / dx;

    int xpx11 = 0;
    double intery = 0;
    {
        double xend = round(x1f);
        double yend = y1f + gradient * (xend - x1f);
        double xgap = 1 - rint(x1f + 0.5);
        xpx11 = xend;
        double ypx11 = rint(yend);

        if (steep) {
            set_pixel_color(
                s, xpx11, ypx11,
                RGB_BRIGHTNESS(color, (1 - (yend - (int)yend)) * xgap * 255));
            set_pixel_color(
                s, xpx11, ypx11 + 1,
                RGB_BRIGHTNESS(color, (yend - (int)yend) * xgap * 255));
        } else {
            set_pixel_color(
                s, ypx11, xpx11,
                RGB_BRIGHTNESS(color, (1 - (yend - (int)yend)) * xgap * 255));
            set_pixel_color(
                s, ypx11 + 1, xpx11,
                RGB_BRIGHTNESS(color, (yend - (int)yend) * xgap * 255));
        }

        intery = yend + gradient;
    }

    int xpx12 = 0;
    {
        double xend = round(x2f);
        double yend = y2f + gradient * (xend - x2f);
        double xgap = 1 - rint(x2f + 0.5);
        xpx12 = xend;
        double ypx12 = rint(yend);

        if (steep) {
            set_pixel_color(
                s, xpx12, ypx12,
                RGB_BRIGHTNESS(color, (1 - (intery - (int)intery)) * 255));
            set_pixel_color(
                s, xpx12, ypx12 + 1,
                RGB_BRIGHTNESS(color, (intery - (int)intery) * 255));
        } else {
            set_pixel_color(
                s, ypx12, xpx12,
                RGB_BRIGHTNESS(color, (1 - (intery - (int)intery)) * 255));
            set_pixel_color(
                s, ypx12 + 1, xpx12,
                RGB_BRIGHTNESS(color, (intery - (int)intery) * 255));
        }
    }

    if (steep) {
        for (size_t x = xpx11 + 1; x < xpx12; x++) {
            set_pixel_color(
                s, x, rint(intery),
                RGB_BRIGHTNESS(color, (1 - (intery - (int)intery)) * 255));
            set_pixel_color(
                s, x, rint(intery) + 1,
                RGB_BRIGHTNESS(color, (intery - (int)intery) * 255));
            intery += gradient;
        }
    } else {
        for (size_t x = xpx11 + 1; x < xpx12; x++) {
            set_pixel_color(
                s, rint(intery), x,
                RGB_BRIGHTNESS(color, (1 - (intery - (int)intery)) * 255));
            set_pixel_color(
                s, rint(intery) + 1, x,
                RGB_BRIGHTNESS(color, (intery - (int)intery) * 255));
            intery += gradient;
        }
    }
}

void draw_rect(void* state, Vec2 p1, Vec2 p2, RGB color) {
    size_t x1 = p1.x;
    size_t y1 = p1.y;
    size_t x2 = p2.x;
    size_t y2 = p2.y;

    draw_vline(state, x1, y1, y2, color);
    draw_vline(state, x2, y1, y2, color);
    draw_hline(state, x1, y1, x2, color);
    draw_hline(state, x1, y2, x2, color);
}

void draw_rect_filled(void* state, Vec2 p1, Vec2 p2, RGB border_color,
                      RGB fill_color) {
    fill_region((SixelState*)state, p1, p2, fill_color);
    draw_rect(state, p1, p2, border_color);
}

void render(void* state) {
    SixelState* s = (SixelState*)state;

    for (size_t row = 0; row < s->height * s->scale; row++) {
        for (size_t col = 0; col < s->width * s->scale; col++) {
            s->upscale[(row * s->width * s->scale) + col] =
                s->pixels[(row / s->scale * s->width) + col / s->scale];
        }
    }

    int status = sixel_encode((void*)s->upscale, s->width * s->scale,
                              s->height * s->scale, 0, s->dither, s->context);
    if (SIXEL_FAILED(status)) {
        fprintf(stderr, "sixel_encode failed during setup\n");
        exit(1);
    }

    memset(s->pixels, 0, s->width * s->height * sizeof(*s->pixels));
}

void cleanup(void* state) {
    SixelState* s = (SixelState*)state;

    sixel_output_destroy(s->context);
    sixel_dither_unref(s->dither);

    fflush(stdout);

    free(s->upscale);
    free(s->pixels);
    free(s);
}

Renderer sx_init(size_t width, size_t height, size_t scale) {
    SixelState* s = (SixelState*)calloc(sizeof(SixelState), 1);

    s->width = width;
    s->height = height;
    s->scale = scale;
    s->pixels = (RGB*)calloc(1, width * height * sizeof(*s->pixels));
    s->upscale =
        (RGB*)calloc(1, width * height * sizeof(*s->pixels) * scale * scale);

    int status = sixel_output_new(&s->context, sixel_write, stdout, NULL);
    if (SIXEL_FAILED(status)) {
        fprintf(stderr, "sixel_output_new failed\n");
        exit(1);
    }

    sixel_output_set_palette_type(s->context, PALETTETYPE_RGB);

    s->dither = sixel_dither_get(SIXEL_BUILTIN_VT340_COLOR);
    sixel_dither_set_pixelformat(s->dither, SIXEL_PIXELFORMAT_RGB888);
    sixel_dither_set_diffusion_type(s->dither, SIXEL_DIFFUSE_X_DITHER);

    status = sixel_encode((void*)s->pixels, s->width, s->height, 0, s->dither,
                          s->context);
    if (SIXEL_FAILED(status)) {
        fprintf(stderr, "sixel_encode failed during setup\n");
        exit(1);
    }

    memset(s->pixels, 0, s->width * s->height * sizeof(*s->pixels));

    return (Renderer){
        .state = s,

        .width = width,
        .height = height,

        .tile_size = FONT_DIM,

        .draw_line = draw_line,
        .draw_rect = draw_rect,
        .draw_rect_filled = draw_rect_filled,
        .draw_text = draw_string,
        .render = render,
        .cleanup = cleanup,
    };
}
