#include "color.h"
#include "events.h"
#include "map.h"
#include "queue.h"
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

    Vec2 tile_pos;
    Vec2 picker_pos;
    bool selected;
    int list_scroll_pos;

    atomic_bool dirty;

    EventQueue events;
    bool done;
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

        /******************/
        /* INPUT HANDLING */
        /******************/
        // We don't want to block rendering on events so we tryget
        EventQueueResult event = lockable_queue_Event_tryget(&ra->events);
        if (event.status == BLOCKED) {
            // There's no background animations for now, so no need to redraw
            // the ui
            goto SLEEP;
        }
        switch (event.item.event_type) {

        case NOP:
            break;
        case UP:
            if (ra->selected) {
                // Tile selection list
                if (ra->picker_pos.y != 0) {
                    // Move normally, we're not at the top
                    ra->picker_pos.y -= 1;
                } else if (ra->picker_pos.y == 0 && ra->list_scroll_pos != 0) {
                    // We're at the bottom of the screen, scroll down
                    ra->list_scroll_pos -= 1;
                }
            } else {
                ra->tile_pos.y =
                    ra->tile_pos.y != 0 ? ra->tile_pos.y - 1 : ra->tile_pos.y;
            }
            break;
        case DOWN:
            if (ra->selected) {
                if (ra->picker_pos.y != 6) {
                    // Move normally, we're not at the bottom
                    ra->picker_pos.y += 1;
                } else if (ra->picker_pos.y == 6) {
                    // We're at the bottom of the screen, scroll down
                    ra->list_scroll_pos += 1;
                }
            } else {
                ra->tile_pos.y =
                    ra->tile_pos.y != 19 ? ra->tile_pos.y + 1 : ra->tile_pos.y;
            }
            break;
        case LEFT:
            if (ra->selected) {
                ra->picker_pos.x = ra->picker_pos.x != 0 ? ra->picker_pos.x - 1
                                                         : ra->picker_pos.x;
            } else {
                ra->tile_pos.x =
                    ra->tile_pos.x != 0 ? ra->tile_pos.x - 1 : ra->tile_pos.x;
            }
            break;
        case RIGHT:
            if (ra->selected) {
                ra->picker_pos.x = ra->picker_pos.x != 11 ? ra->picker_pos.x + 1
                                                          : ra->picker_pos.x;
            } else {
                ra->tile_pos.x =
                    ra->tile_pos.x != 29 ? ra->tile_pos.x + 1 : ra->tile_pos.x;
            }
            break;
        case ENTER:
            ra->selected = true;
            break;
        case ESCAPE:
            ra->selected = false;
            break;
        case QUIT:
            ra->done = true;
            goto CLEANUP;
            break;
        }

        /*************/
        /* RENDERING */
        /*************/
        ra->r.draw_map(ra->r.state, ra->m);

        // Draw red outline
        ra->r.draw_rect(ra->r.state,
                        tile_coords(ra->tile_pos, ra->t.tile_dimensions.x, NW),
                        tile_coords(ra->tile_pos, ra->t.tile_dimensions.x, SE),
                        (RGBA){.r = 0xFF, .a = ra->selected ? 0xEF : 0x95});

        // If selected we want to pop up the tile picker
        if (ra->selected) {
            ra->r.draw_rect_filled(
                ra->r.state,
                tile_coords(VEC2(2, 2), ra->t.tile_dimensions.x, NW),
                tile_coords(VEC2(27, 17), ra->t.tile_dimensions.x, SE), WHITE,
                (RGBA){.r = 0xAF, .g = 0xAF, .b = 0xAF, .a = 0xAF});

            const size_t tiles_per_row = ((27 - 3) / 2);
            const int rows_of_tiles = ra->t.num_tiles / tiles_per_row;
            const int num_rendered_rows = (17 - 3) / 2;

            // First check if they attempted to scroll past the end
            if (ra->list_scroll_pos >=
                rows_of_tiles - (num_rendered_rows - 1)) {
                ra->list_scroll_pos = rows_of_tiles - (num_rendered_rows - 1);
            }

            // When the user does scroll down the last row of tiles may be
            // incomplete, check this and move the the last tile if so
            if (ra->list_scroll_pos ==
                    rows_of_tiles - (num_rendered_rows - 1) &&
                ra->picker_pos.y == num_rendered_rows - 1) {
                // Check how many tiles are being rendered in the last row
                const int last_row_tiles = ra->t.num_tiles % tiles_per_row;

                if (ra->picker_pos.x >= last_row_tiles) {
                    ra->picker_pos.x = last_row_tiles - 1;
                }
            }

            // Now draw all of the tiles that will fit
            int starting_tile_index = ra->list_scroll_pos * tiles_per_row;
            Vec2 tile =
                VEC2(starting_tile_index % (int)(ra->t.dimensions_in_tiles.x),
                     (int)(starting_tile_index /
                           (int)(ra->t.dimensions_in_tiles.x)));

            for (size_t row = 3; row < 17; row += 2) {
                for (size_t col = 3; col < 27; col += 2) {
                    // Don't draw past the end of the tilemap
                    if ((tile.y *
                         (ra->t.dimensions.x / ra->t.tile_dimensions.x)) +
                            tile.x >=
                        ra->t.num_tiles) {
                        break;
                    }

                    ra->r.draw_tile(ra->r.state, VEC2(col, row), VEC2(4, 4),
                                    ra->t, tile, 0, 0);

                    tile.x++;
                    if (tile.x ==
                        ra->t.dimensions.x / ra->t.tile_dimensions.x) {
                        tile.x = 0;
                        tile.y++;
                    }
                }
            }

            // Draw tile picker
            Vec2 picker_draw_nw = vec2_add(
                tile_coords(vec2_add(vec2_mul(ra->picker_pos, 2), VEC2(3, 3)),
                            ra->t.tile_dimensions.x, NW),
                3);
            Vec2 picker_draw_se = vec2_add(
                tile_coords(vec2_add(vec2_mul(ra->picker_pos, 2), VEC2(3, 3)),
                            ra->t.tile_dimensions.x, SE),
                5);
            ra->r.draw_rect(ra->r.state, picker_draw_nw, picker_draw_se,
                            (RGBA){.r = 0xFF, .g = 0xFF, .b = 0x00, .a = 0x95});
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

CLEANUP:
    return NULL;
}

LockableQueuePrototypes(char);
LockableQueueImpl(char);

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
    Renderer r = sx_init(240, 160, 3);

    RenderArgs ra = {
        .r = r,
        .t = t,
        .m = (Map){.tiles = (Vec2*)map, .t = t, .dimensions = VEC2(30, 20)},

        .tile_pos = {0},
        .picker_pos = {0},
        .selected = false,
        .list_scroll_pos = 0,

        .dirty = true,

        .events = lockable_queue_Event_init(1),
        .done = false,
    };
    pthread_t thread;
    pthread_create(&thread, NULL, render_thread, &ra);

    enableRawMode();

    while (!ra.done) {
        EventQueueResult result;
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
        Event e = (Event){.event_type = NOP};
        switch (c) {
        case 13:
            e.event_type = ENTER;
            break;
        case 27:
            e.event_type = ESCAPE;
        case 'h':
            e.event_type = LEFT;
            break;
        case 'j':
            e.event_type = DOWN;
            break;
        case 'k':
            e.event_type = UP;
            break;
        case 'l':
            e.event_type = RIGHT;
            break;
        case 'q':
            e.event_type = QUIT;
        default:
            break;
        }

        result = lockable_queue_Event_add(&ra.events, e);
    }

CLEANUP:
    pthread_join(thread, NULL);
    printf("\n");
    r.cleanup(r.state);
    tilemap_deinit(t);
}
