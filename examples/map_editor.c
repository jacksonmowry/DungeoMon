#include "color.h"
#include "map.h"
#include "renderer.h"
#include "sprites.h"
#include "sx.h"
#include "tile.h"
#include "timespec.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
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

typedef struct RenderArgs {
    Renderer r;
    Map m;
    Tilemap t;

    Vec2 pos;
    bool selected;
    atomic_bool dirty;
} RenderArgs;

void* render_thread(void* arg) {
    RenderArgs* ra = (RenderArgs*)arg;

    struct timespec frame_time = timespec_from_double(1 / (double)30);

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

        if (!ra->dirty) {
            goto SLEEP;
        }

        ra->r.draw_map(ra->r.state, ra->m);

        // Draw red outline
        ra->r.draw_rect(ra->r.state,
                        tile_coords(ra->pos, ra->t.tile_dimensions.x, NW),
                        tile_coords(ra->pos, ra->t.tile_dimensions.x, SE),
                        (RGBA){.r = 0xFF, .a = ra->selected ? 0xEF : 0x95});

        // If selected we want to pop up the tile picker
        if (ra->selected) {
            ra->r.draw_rect_filled(
                ra->r.state,
                tile_coords(VEC2(2, 2), ra->t.tile_dimensions.x, NW),
                tile_coords(VEC2(27, 17), ra->t.tile_dimensions.x, SE), WHITE,
                (RGBA){.r = 0xAF, .g = 0xAF, .b = 0xAF, .a = 0xAF});
        }

        printf("\033[H");
        fflush(stdout);
        ra->r.render(ra->r.state);
        printf("\n");
        printf("'q' to quit, 'hjkl' to move left down up right, 'Enter' to "
               "select a tile, 'Escape' to unselect a tile\n");
        /* ra->dirty = false; */

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

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        fprintf(stderr,
                "usage: %s sprite_sheet.png tile_width tile_height "
                "horizontal_gap vertical_gap\n",
                argv[0]);
        return 1;
    }

    srand(time(NULL));

    Vec2 tile_dims = VEC2(atoi(argv[2]), atoi(argv[3]));
    Vec2 tile_gaps = VEC2(atoi(argv[4]), atoi(argv[5]));
    Tilemap t = tilemap_load_png(argv[1], tile_dims, tile_gaps);

    // clang-format off
    Vec2 map[20][30] = {
        {WALL_TOP_LEFT, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP, WALL_TOP_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_LEFT, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WALL_RIGHT},
        {WALL_BOTTOM_LEFT, WALL_BOTTOM_1, WALL_BOTTOM_2, WALL_BOTTOM_1, WALL_BOTTOM_1, WALL_BOTTOM_2, WALL_BOTTOM_1, WALL_BOTTOM_2, WALL_BOTTOM_1, WALL_BOTTOM_2, WALL_BOTTOM_2, WALL_BOTTOM_1, WALL_BOTTOM_2, WALL_BOTTOM_1, WALL_BOTTOM_1, WALL_BOTTOM_2, WALL_BOTTOM_1, WALL_BOTTOM_2, WALL_BOTTOM_1, WALL_BOTTOM_2, WALL_BOTTOM_2, WALL_BOTTOM_1, WALL_BOTTOM_2, WALL_BOTTOM_1, WALL_BOTTOM_1, WALL_BOTTOM_2, WALL_BOTTOM_1, WALL_BOTTOM_2, WALL_BOTTOM_1, WALL_BOTTOM_RIGHT}
    };
    // clang-format on
    Renderer r = sx_init(240, 160, 6);

    RenderArgs ra = {
        .r = r,
        .t = t,
        .m = (Map){.tiles = (Vec2*)map, .t = t, .dimensions = VEC2(30, 20)},

        .pos = {},
        .selected = false,
        .dirty = true,
    };
    pthread_t thread;
    pthread_create(&thread, NULL, render_thread, &ra);

    enableRawMode();

    while (1) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
        if (iscntrl(c)) {
            if (c == 13) {
                // Enter
                ra.selected = true;
            } else if (c == 27) {
                // Escapse
                ra.selected = false;
            }
        } else if (!ra.selected) {
            // We can't enter this block unless the tile is unselected
            switch (c) {
            case 'h':
                ra.pos.x = ra.pos.x != 0 ? ra.pos.x - 1 : ra.pos.x;
                ra.selected = false;
                break;
            case 'j':
                ra.pos.y = ra.pos.y != 19 ? ra.pos.y + 1 : ra.pos.y;
                ra.selected = false;
                break;
            case 'k':
                ra.pos.y = ra.pos.y != 0 ? ra.pos.y - 1 : ra.pos.y;
                ra.selected = false;
                break;
            case 'l':
                ra.pos.x = ra.pos.x != 29 ? ra.pos.x + 1 : ra.pos.x;
                ra.selected = false;
                break;
            default:
                break;
            }
            ra.dirty = true;
        } else {
            // Quitting should always work
            switch (c) {
            case 'q':
                goto CLEANUP;
            }
        }
    }

CLEANUP:
    pthread_cancel(thread);
    pthread_join(thread, NULL);
    printf("\n");
    r.cleanup(r.state);
    tilemap_deinit(t);
}
