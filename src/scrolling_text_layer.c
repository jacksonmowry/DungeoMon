#include "scrolling_text_layer.h"
#include "layer.h"
#include <assert.h>
#include <ctype.h>
#include <string.h>

typedef struct ScrollingTextLayerState {
    char message_lines[32][19];
    size_t num_lines;
    size_t current_line;
} ScrollingTextLayerState;

static LayerEventResponse handle_input(void* state, Event e) {
    ScrollingTextLayerState* s = (ScrollingTextLayerState*)state;

    LayerEventResponse response = {0};

    switch (e.event_type) {
    case ENTER:
        // Scrolling logic here, HANLDED if we have text left, POP if done
        response.status = HANDLED;
    default:
        response.status = IGNORED;
    }

    return response;
}

static void render(void* state) {
    ScrollingTextLayerState* s = (ScrollingTextLayerState*)state;
    return;
}

static void deinit(void* state) {
    ScrollingTextLayerState* s = (ScrollingTextLayerState*)state;

    for (size_t i = 0; i < 64; i++) {
        free(s->message_lines[i]);
    }
}

Layer scrolling_text_layer_init(Renderer* r, const char* message) {
    ScrollingTextLayerState* s = calloc(1, sizeof(*s));

    // Custom line break algorithm, we can have a max of 18 characters per line
    char word_buf[18] = {0};
    size_t word_len = 0;
    const size_t message_len = strlen(message);
    size_t line_len = 0;

    for (size_t i = 0; i < message_len + 1; i++) {
        char c = i == message_len ? ' ' : message[i];

        if (isspace(c)) {
            fprintf(stderr, "Space, word is \"%s\"\n", word_buf);
            // Word break, push current word onto line if it fits
            assert(word_len <= 18);
            if (line_len + word_len < 18) {
                word_buf[word_len] = ' ';
                strcat(s->message_lines[s->num_lines], word_buf);
                line_len += word_len;

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
            fprintf(stderr, "Plain char: '%c'\n", c);
            word_buf[word_len++] = c;
        }
    }

    // One extra line
    s->num_lines++;

    fprintf(stderr, "Message: \"%s\", translates into %zu lines: \n", message,
            s->num_lines);

    for (size_t i = 0; i < s->num_lines; i++) {
        fprintf(stderr, "%s\n", s->message_lines[i]);
    }

    return (Layer){
        .state = s,

        .handle_input = handle_input,
        .render = render,
        .deinit = deinit,
    };
}
