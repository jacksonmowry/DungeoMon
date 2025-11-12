#include "renderer.h"
#include "sx.h"
#include "tile.h"
#include "vec.h"
#include <float.h>
#include <getopt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    const size_t tile_dim = 8;
    const size_t width = 240;
    const size_t height = 160;

    const size_t width_in_tiles = width / tile_dim;
    const size_t height_in_tiles = height / tile_dim;

    size_t render_scale = 2;
    const size_t font_scale = 1;

    // Update render scale is user requests
    if (argc > 1) {
        sscanf(argv[1], "%zu", &render_scale);
    }

    Renderer r = sx_init(width, height, render_scale);
    // A simple white box to show the outline of our render
    r.draw_rect(r.state, (Vec2I){.x = 0, .y = 0},
                tile_coords(VEC2I(width_in_tiles - 1, height_in_tiles - 1),
                            tile_dim, SE),
                RGBA(WHITE));

    // We're starting both x&y at 1 to leave padding between the character
    // and the border
    Vec2I pos = {.x = 1, .y = 1};
    for (char c = '!'; c <= '~'; c++) {
        char s[2] = {c, '\0'};
        r.draw_text(r.state, s, tile_coords(pos, tile_dim, NW), RGBA(WHITE), 1);

        pos.x++;
        if (pos.x >= width_in_tiles - 1) {
            pos.x = 1;
            pos.y++;

            if (pos.y >= height_in_tiles) {
                fprintf(stderr, "Out of space on the screen\n");
                goto cleanup;
            }
        }
    }

    r.render(r.state);

cleanup:
    r.cleanup(r.state);
    printf("\n");
}
