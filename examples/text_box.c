#include "events.h"
#include "layer.h"
#include "queue.h"
#include "renderer.h"
#include "scrolling_text_layer.h"
#include "sx.h"
#include "timespec.h"
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
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

    EventQueue events;
    bool done;
} RenderArgs;

void* render_thread(void* arg) {
    RenderArgs* ra = (RenderArgs*)arg;

    Layer* layer_stack[3];
    int layers = 0;
    Layer text_box =
        scrolling_text_layer_init(&ra->r,
                                  "You encountered a goblin! How does it "
                                  "handle a bit longer of a phrase?",
                                  true);
    layer_stack[0] = &text_box;
    layers = 1;

#ifdef DEBUG
    Layer* debug_layer_stack[3];
    int debug_layers = 0;
#endif

    struct timespec frame_time = timespec_from_double(1 / (double)30);

    struct timespec frame_prev = {0};
    if (clock_gettime(CLOCK_REALTIME, &frame_prev) == -1) {
        perror("clock_gettime");
        exit(1);
    }
    frame_prev = timespec_sub(frame_prev, timespec_from_double(1 / 30.0));

    while (layers != 0) {
        struct timespec frame_start = {0};
        if (clock_gettime(CLOCK_REALTIME, &frame_start) == -1) {
            perror("clock_gettime");
            exit(1);
        }
        struct timespec next_frame = timespec_add(frame_start, frame_time);
        double framerate =
            1 / timespec_to_double(timespec_sub(frame_start, frame_prev));

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

        if (event.item.event_type == QUIT) {
            // Force quit the game
            goto CLEANUP;
        }

#ifdef DEBUG
    DEBUG_INPUT_LOOP:
        for (int i = debug_layers - 1; i >= 0; i--) {
            LayerEventResponse response = debug_layer_stack[i]->handle_input(
                debug_layer_stack[i]->state, event.item);

            switch (response.status) {
            case IGNORED:
                continue;
            case HANDLED:
                goto RENDER_LOOP;
            case POP:
                assert(false); // Debug layers cannot pop regular layers
                break;
            case PUSH:
                assert(false); // Debug layers cannot push regular layers
                break;
            case DEBUG_POP:
                assert(false); // Not currently handled
                break;
            case DEBUG_PUSH:
                assert(false); // Not currently handled
                break;
            }
        }
#endif

        for (int i = layers - 1; i >= 0; i--) {
            LayerEventResponse response =
                layer_stack[i]->handle_input(layer_stack[i]->state, event.item);

            switch (response.status) {
            case IGNORED:
                continue;
            case HANDLED:
                goto RENDER_LOOP;
            case POP:
                event.item.event_type = POPPED;
                if (i == 0) {
                    goto CLEANUP;
                }

                layer_stack[i]->deinit(layer_stack[i]->state);
                layers--;
                break;
            case PUSH:
                layer_stack[layers] = response.l;
                layers++;
                goto RENDER_LOOP;
#ifdef DEBUG
            case DEBUG_POP:
                event.item.event_type = POPPED;

                debug_layer_stack[i]->deinit(debug_layer_stack[i]->state);
                debug_layers--;
                break;
            case DEBUG_PUSH:
                debug_layer_stack[debug_layers] = response.l;
                debug_layers++;
                break;
#endif
            }
        }

    RENDER_LOOP:
        for (int i = 0; i < layers; i++) {
            layer_stack[i]->render(layer_stack[i]->state);
        }
#ifdef DEBUG
        for (int i = 0; i < debug_layers; i++) {
            debug_layer_stack[i]->render(debug_layer_stack[i]->state);
        }
#endif

        printf("\033[H");
        fflush(stdout);
        ra->r.render(ra->r.state);
        printf("\n");
        printf("FPS %.2f\r\n", framerate);

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
#ifdef DEBUG
    for (int i = debug_layers - 1; i >= 0; i--) {
        debug_layer_stack[i]->deinit(debug_layer_stack[i]->state);
    }
#endif
    for (int i = layers - 1; i >= 0; i--) {
        layer_stack[i]->deinit(layer_stack[i]->state);
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    const size_t width = 240;
    const size_t height = 160;
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    // figure out how much we can upscale based on the terminal size
    size_t horizontal_scale = w.ws_xpixel / width;
    size_t vertical_scale = w.ws_ypixel / height;
    size_t scale =
        horizontal_scale < vertical_scale ? horizontal_scale : vertical_scale;

    Renderer r = sx_init(width, height, scale);

    RenderArgs ra = {
        .r = r,

        .events = lockable_queue_Event_init(1),
        .done = false,
    };

    pthread_t thread;
    pthread_create(&thread, NULL, render_thread, &ra);

    enableRawMode();

    struct timespec frame_time = timespec_from_double(1 / (double)60);

    struct timespec frame_prev = {0};
    if (clock_gettime(CLOCK_REALTIME, &frame_prev) == -1) {
        perror("clock_gettime");
        exit(1);
    }

    while (!ra.done) {
        struct timespec frame_start = {0};
        if (clock_gettime(CLOCK_REALTIME, &frame_start) == -1) {
            perror("clock_gettime");
            exit(1);
        }
        struct timespec next_frame = timespec_add(frame_start, frame_time);

        // Window resizing
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        // figure out how much we can upscale based on the terminal size
        size_t horizontal_scale = w.ws_xpixel / width;
        size_t vertical_scale = w.ws_ypixel / height;
        size_t scale = horizontal_scale < vertical_scale ? horizontal_scale
                                                         : vertical_scale;
        if (scale != ra.r.scale) {
            ra.r.update_scale(ra.r.state, scale);
        }

        // User keypresses
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
            die("read");
        }

        Event e = (Event){.event_type = NOP};
        switch (c) {
        case 13:
            e.event_type = ENTER;
            break;
        case 27:
            e.event_type = ESCAPE;
            break;
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
        case 'r':
            e.event_type = R;
            break;
        case 'v':
            e.event_type = V;
            break;
        case 'b':
            e.event_type = B;
            break;
        case 'd':
            e.event_type = D;
            break;
        case 't':
            e.event_type = T;
            break;
        case 'w':
            e.event_type = W;
            break;
        case 'q':
            e.event_type = QUIT;
            break;
        default:
            goto SLEEP;
            break;
        }

        EventQueueResult result = lockable_queue_Event_tryadd(&ra.events, e);
        if (result.status == BLOCKED) {
            ;
        }

        if (c == 'q') {
            goto CLEANUP;
        }

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
    pthread_join(thread, NULL);
    printf("\n");
    r.cleanup(r.state);
    lockable_queue_Event_deinit(ra.events);
}
