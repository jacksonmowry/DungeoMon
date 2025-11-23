#include "renderer.h"
#include "sx.h"
#include "tile.h"
#include "timespec.h"
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
#include <sys/time.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    const size_t tile_dim = 8;
    const size_t width = 240;
    const size_t height = 160;

    const size_t width_in_tiles = width / tile_dim;
    const size_t height_in_tiles = height / tile_dim;

    size_t render_scale = 2;

    // Update render scale is user requests
    if (argc > 1) {
        sscanf(argv[1], "%zu", &render_scale);
    }

    Renderer r = sx_init(width, height, render_scale);

    // Red rectangle to move around, this will just be 1 tile big
    // Coordinates are in "tile space"
    // Velocity is initialized to (1,1), meaning it will move down and to the
    // right
    Vec2I red_box =
        VEC2I((double)width_in_tiles / 2, (double)height_in_tiles / 2);
    Vec2I red_box_velocity = VEC2I_SPLAT(1);

    // 15FPS
    struct timespec frame_time = timespec_from_double(1 / (double)15);

    while (true) {
        struct timespec frame_start = timespec_get_time();
        struct timespec next_frame = timespec_add(frame_start, frame_time);

        printf("\033[H");
        fflush(stdout);
        // A simple white box to show the outline of our render
        r.draw_rect(r.state, (Vec2I){.x = 0, .y = 0},
                    tile_coords(VEC2I(width_in_tiles - 1, height_in_tiles - 1),
                                tile_dim, SE),
                    RGBA(WHITE));

        // Draw red rectangle, grap upper left (NW) and lower right (SE) coords
        r.draw_rect_filled(r.state, tile_coords(red_box, tile_dim, NW),
                           tile_coords(red_box, tile_dim, SE), RGBA(RED),
                           RGBA(RED));

        // Check if would collide, if so update velocity
        Vec2I new_pos = vec2i_add(red_box, red_box_velocity);
        bool horizontal_collision =
            new_pos.x < 0 || (size_t)new_pos.x >= width_in_tiles;
        bool vertical_collision =
            new_pos.y < 0 || (size_t)new_pos.y >= height_in_tiles;
        red_box_velocity =
            vec2i_mul(red_box_velocity, VEC2I(horizontal_collision ? -1 : 1,
                                              vertical_collision ? -1 : 1));
        // Update position
        red_box = vec2i_add(red_box, red_box_velocity);

        // Currently render clears the pixel buffer
        r.render(r.state);

        struct timespec frame_end = timespec_get_time();

        struct timespec sleep_length = timespec_sub(next_frame, frame_end);
        thrd_sleep(&sleep_length, NULL);
    }

    r.cleanup(r.state);
    printf("\n");
}
