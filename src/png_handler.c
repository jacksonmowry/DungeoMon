#include "png_handler.h"
#include "color.h"

/*
 * A simple libpng example program
 * http://zarb.org/~gc/html/libpng.html
 *
 * Modified by Yoshimasa Niwa to make it much simpler
 * and support all defined color_type.
 *
 * To build, use the next instruction on OS X.
 * $ brew install libpng
 * $ clang -lz -lpng16 libpng_test.c
 *
 * Copyright 2002-2010 Guillaume Cottenceau.
 *
 * This software may be freely redistributed under the terms
 * of the X11 license.
 *
 */

#include <fcntl.h>
#include <png.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

RGBA* read_pam_file(const char* filename, Vec2I* dimensions) {
    int width, height;

    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror(filename);
        exit(1);
    }

    char buf[256];

    // P7
    fgets(buf, sizeof(buf), fp);
    if (strncmp("P7", buf, 2)) {
        fprintf(stderr,
                "Error parsing PAM image file, expecting: P7, got: %s\n", buf);
        exit(1);
    }
    // WIDTH x
    fgets(buf, sizeof(buf), fp);
    if (strncmp("WIDTH", buf, 5)) {
        fprintf(stderr,
                "Error parsing PAM image file, expecting: WIDTH x, got: %s\n",
                buf);
        exit(1);
    }
    if (sscanf(buf, "WIDTH %d", &width) != 1) {
        fprintf(stderr,
                "Error parsing PAM image file, expecting: WIDTH x, got: %s\n",
                buf);
        exit(1);
    }
    // HEIGHT y
    fgets(buf, sizeof(buf), fp);
    if (strncmp("HEIGHT", buf, 6)) {
        fprintf(stderr,
                "Error parsing PAM image file, expecting: HEIGHT y, got: %s\n",
                buf);
        exit(1);
    }
    if (sscanf(buf, "HEIGHT %d", &height) != 1) {
        fprintf(stderr,
                "Error parsing PAM image file, expecting: HEIGHT y, got: %s\n",
                buf);
        exit(1);
    }
    // DEPTH 4
    fgets(buf, sizeof(buf), fp);
    if (strncmp("DEPTH 4", buf, 7)) {
        fprintf(stderr,
                "Error parsing PAM image file, expecting: DEPTH 4, got: %s\n",
                buf);
        exit(1);
    }
    // MAXVAL 255
    fgets(buf, sizeof(buf), fp);
    if (strncmp("MAXVAL 255", buf, 10)) {
        fprintf(
            stderr,
            "Error parsing PAM image file, expecting: MAXVAL 255, got: %s\n",
            buf);
        exit(1);
    }
    // TUPLTYPE RGB_ALPHA
    fgets(buf, sizeof(buf), fp);
    if (strncmp("TUPLTYPE RGB_ALPHA", buf, 18)) {
        fprintf(stderr,
                "Error parsing PAM image file, expecting: TUPLTYPE "
                "RGB_ALPHA, got: %s\n",
                buf);
        exit(1);
    }
    // ENDHDR
    fgets(buf, sizeof(buf), fp);
    if (strncmp("ENDHDR", buf, 6)) {
        fprintf(stderr,
                "Error parsing PAM image file, expecting: ENDHDR, got: %s\n",
                buf);
        exit(1);
    }

    RGBA* pixels = malloc(width * height * sizeof(*pixels));
    for (size_t row = 0; row < height; row++) {
        for (size_t col = 0; col < width; col++) {
            size_t index = (row * width) + col;

            fread(pixels + index, sizeof(uint32_t), 1, fp);
        }
    }
    fclose(fp);

    dimensions->x = width;
    dimensions->y = height;
    return pixels;
}

RGBA* read_png_file(const char* filename, Vec2I* dimensions) {
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep* row_pointers = NULL;

    FILE* fp = fopen(filename, "rb");

    png_structp png =
        png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
        abort();

    png_infop info = png_create_info_struct(png);
    if (!info)
        abort();

    if (setjmp(png_jmpbuf(png)))
        abort();

    png_init_io(png, fp);

    png_read_info(png, info);

    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    color_type = png_get_color_type(png, info);
    bit_depth = png_get_bit_depth(png, info);

    // Read any color_type into 8bit depth, RGBA format.
    // See http://www.libpng.org/pub/png/libpng-manual.txt

    if (bit_depth == 16)
        png_set_strip_16(png);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    // These color_type don't have an alpha channel then fill it with 0xff.
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    if (row_pointers)
        abort();

    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
    }

    png_read_image(png, row_pointers);

    fclose(fp);

    png_destroy_read_struct(&png, &info, NULL);

    RGBA* pixels = malloc(width * height * sizeof(*pixels));
    for (int y = 0; y < height; y++) {
        png_bytep row = row_pointers[y];
        for (int x = 0; x < width; x++) {
            png_bytep px = &(row[x * 4]);
            pixels[(y * width) + x] =
                (RGBA){.r = px[0], .g = px[1], .b = px[2], .a = px[3]};
            // Do something awesome for each pixel here...
            // printf("%4d, %4d = RGBA(%3d, %3d, %3d, %3d)\n", x, y, px[0],
            // px[1], px[2], px[3]);
        }
    }

    dimensions->x = width;
    dimensions->y = height;
    for (int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);
    return pixels;
}
