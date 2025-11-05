#include "renderer.h"
#include "sx.h"
#include "tile.h"
#include "timespec.h"
#include "vec.h"
#include <ctype.h>
#include <errno.h>
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
#include <termios.h>
#include <time.h>
#include <unistd.h>

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

void die(const char* s) {
    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

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

    enableRawMode();

    while (1) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q')
            break;
    }

    return 0;

    Renderer r = sx_init(width, height, render_scale);

    // Red rectangle to move around, this will just be 1 tile big
    // Coordinates are in "tile space"
    // Velocity is initialized to (1,1), meaning it will move down and to the
    // right
    Vec2 red_box =
        VEC2((double)width_in_tiles / 2, (double)height_in_tiles / 2);
    Vec2 red_box_velocity = VEC2_SPLAT(1);

    // 15FPS
    struct timespec frame_time = timespec_from_double(1 / (double)15);

    struct timespec frame_prev = {0};
    if (clock_gettime(CLOCK_REALTIME, &frame_prev) == -1) {
        perror("clock_gettime");
        exit(1);
    }

    while (true) {
        struct timespec frame_start = {0};
        if (clock_gettime(CLOCK_REALTIME, &frame_start) == -1) {
            perror("clock_gettime");
            exit(1);
        }
        struct timespec next_frame = timespec_add(frame_start, frame_time);

        printf("\033[H");
        fflush(stdout);
        // A simple white box to show the outline of our render
        r.draw_rect(r.state, (Vec2){.x = 0, .y = 0},
                    tile_coords(VEC2(width_in_tiles - 1, height_in_tiles - 1),
                                tile_dim, SE),
                    WHITE);

        // Draw red rectangle, grap upper left (NW) and lower right (SE) coords
        r.draw_rect_filled(r.state, tile_coords(red_box, tile_dim, NW),
                           tile_coords(red_box, tile_dim, SE), RED, RED);

        // Check if would collide, if so update velocity
        Vec2 new_pos = vec2_add(red_box, red_box_velocity);
        bool horizontal_collision =
            new_pos.x < 0 || new_pos.x >= width_in_tiles;
        bool vertical_collision = new_pos.y < 0 || new_pos.y >= height_in_tiles;
        red_box_velocity =
            vec2_mul(red_box_velocity, VEC2(horizontal_collision ? -1 : 1,
                                            vertical_collision ? -1 : 1));
        // Update position
        red_box = vec2_add(red_box, red_box_velocity);

        // Currently render clears the pixel buffer
        r.render(r.state);

    SLEEP:;
        struct timespec frame_end = {0};
        if (clock_gettime(CLOCK_REALTIME, &frame_end) == -1) {
            perror("clock_gettime");
            exit(1);
        }

        frame_prev = frame_start;

        struct timespec sleep_length = timespec_sub(next_frame, frame_end);
        nanosleep(&sleep_length, NULL);
    }

cleanup:
    r.cleanup(r.state);
    printf("\n");
}
