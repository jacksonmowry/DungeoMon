#include "scrolling_text_layer.h"
#include "layer.h"
#include "tile.h"
#include <assert.h>
#include <ctype.h>
#include <string.h>

#define MESSAGE_LINE_LENGTH 25

typedef struct ScrollingTextLayerState {
    Renderer* r;

    char message_lines[32][MESSAGE_LINE_LENGTH + 1];
    size_t num_lines;
    size_t current_line;

    bool top;
} ScrollingTextLayerState;

static LayerEventResponse handle_input(void* state, Event e) {
    assert(state);

    ScrollingTextLayerState* s = (ScrollingTextLayerState*)state;

    LayerEventResponse response = {0};

    switch (e.event_type) {
    case ENTER:
        // Scrolling logic here, HANLDED if we have text left, POP if done
        if (s->current_line != s->num_lines - 2) {
            // We still have room to scroll
            s->current_line++;
            response.status = HANDLED;
        } else {
            // We're done, pop ourselves
            response.status = POP;
        }
        break;
    default:
        response.status = IGNORED;
    }

    return response;
}

static void render(void* state) {
    assert(state);

    ScrollingTextLayerState* s = (ScrollingTextLayerState*)state;

    if (s->top) {
        s->r->draw_rect_filled(s->r->state, tile_coords(VEC2I(1, 2), 8, NW),
                               tile_coords(VEC2I(28, 5), 8, SE), WHITE, BLACK);

        s->r->draw_text(s->r->state, s->message_lines[s->current_line],
                        tile_coords(VEC2I(2, 3), 8, NW), WHITE, 1);
        s->r->draw_text(s->r->state, s->message_lines[s->current_line + 1],
                        tile_coords(VEC2I(2, 4), 8, NW), WHITE, 1);

        // Do we still have more lines to show? If yes show scroll indicator
        if (s->current_line != s->num_lines - 2) {
            s->r->draw_text(s->r->state, "!", tile_coords(VEC2I(27, 4), 8, NW),
                            WHITE, 1);
        }
    } else {
        s->r->draw_rect_filled(s->r->state, tile_coords(VEC2I(1, 14), 8, NW),
                               tile_coords(VEC2I(28, 17), 8, SE), WHITE, BLACK);

        s->r->draw_text(s->r->state, s->message_lines[s->current_line],
                        tile_coords(VEC2I(2, 15), 8, NW), WHITE, 1);
        s->r->draw_text(s->r->state, s->message_lines[s->current_line + 1],
                        tile_coords(VEC2I(2, 16), 8, NW), WHITE, 1);

        // Do we still have more lines to show? If yes show scroll indicator
        if (s->current_line != s->num_lines - 2) {
            s->r->draw_text(s->r->state, "!", tile_coords(VEC2I(27, 16), 8, NW),
                            WHITE, 1);
        }
    }

    return;
}

static void deinit(void* state) {
    assert(state);

    ScrollingTextLayerState* s = (ScrollingTextLayerState*)state;

    free(s);
}

Layer scrolling_text_layer_init(Renderer* r, const char* message, bool top) {
    assert(r);
    assert(message);

    ScrollingTextLayerState* s = calloc(1, sizeof(*s));
    assert(s);

    s->top = top;
    s->r = r;

    // Custom line break algorithm, we can have a max of 18 characters per line
    char word_buf[MESSAGE_LINE_LENGTH] = {0};
    size_t word_len = 0;
    const size_t message_len = strlen(message);
    size_t line_len = 0;

    for (size_t i = 0; i < message_len + 1; i++) {
        const char c = i == message_len ? ' ' : message[i];

        if (isspace(c)) {
            word_buf[word_len++] = ' ';
            // Word break, push current word onto line if it fits
            assert(word_len <= MESSAGE_LINE_LENGTH);
            if (line_len + word_len <= MESSAGE_LINE_LENGTH) {
                strcat(s->message_lines[s->num_lines], word_buf);
                line_len += word_len;

                assert(line_len <= MESSAGE_LINE_LENGTH);
                assert(strlen(s->message_lines[s->num_lines]) <=
                       MESSAGE_LINE_LENGTH);
            } else {
                // We need a new line
                line_len = 0;
                s->num_lines++;

                strcpy(s->message_lines[s->num_lines], word_buf);
                line_len += word_len;
            }
            memset(word_buf, 0, sizeof(word_buf));
            word_len = 0;
        } else {
            word_buf[word_len++] = c;
        }
    }

    // One extra line
    s->num_lines++;

    return (Layer){
        .state = s,

        .handle_input = handle_input,
        .render = render,
        .deinit = deinit,
    };
}
