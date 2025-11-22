#include "debug_layer.h"
#include "layer.h"
#include "map.h"
#include "renderer.h"
#include <assert.h>

typedef struct DebugState {
    Renderer* r;
    const Map* m;
    const Vec2I* tile_pos;

    int attribute_overlay;
} DebugState;

static LayerEventResponse handle_input(void* state, Event e) {
    assert(state);

    DebugState* s = (DebugState*)state;

    LayerEventResponse response = {0};
    // Handle
    switch (e.event_type) {
    case T:
        s->attribute_overlay = (s->attribute_overlay + 1) % TILE_NUM_ATTRIBUTES;
        response.status = HANDLED;
        break;
    default:
        response.status = IGNORED;
        break;
    }

    return response;
}

static void render(void* state) {
    assert(state);

    DebugState* s = (DebugState*)state;

    if (s->attribute_overlay != TILE_NORMAL) {
        // Draw a blue tint over all tiles with the currently selected
        // attribute
        for (size_t row = 0; row < (size_t)s->m->dimensions.y; row++) {
            for (size_t col = 0; col < (size_t)s->m->dimensions.x; col++) {
                size_t index = (row * s->m->dimensions.x) + col;
                if (s->m->tile_attributes[index] &
                    (1 << (s->attribute_overlay - 1))) {
                    Vec2I pos = VEC2I(col, row);
                    s->r->draw_rect_filled(
                        s->r->state,
                        tile_coords(pos, s->m->t->tile_dimensions.x, NW),
                        tile_coords(pos, s->m->t->tile_dimensions.x, SE),
                        (RGBA){.r = 0x00, .b = 0xFF, .g = 0x00, .a = 0x45},
                        (RGBA){.r = 0x00, .b = 0xFF, .g = 0x00, .a = 0x45});
                }
            }
        }
    }

    static char buf[4096];
    printf("\033[0J");
    printf("Overlay highlight: ");
    switch (s->attribute_overlay) {
    case 1:
        printf(TILE_HORIZONTAL_FLIP_STR);
        break;
    case 2:
        printf(TILE_VERTICAL_FLIP_STR);
        break;
    case 3:
        printf(TILE_WALL_STR);
        break;
    case 4:
        printf(TILE_ENEMY_STR);
        break;
    case 5:
        printf(TILE_DOOR_STR);
        break;
    case 6:
        printf(TILE_STAIRS_STR);
        break;
    case 7:
        printf(TILE_LOOT_STR);
        break;
    default:
        printf("none");
    }
    printf("\r\n");
    map_tile_attributes_debug(s->m, *s->tile_pos, buf, 4096);
    printf("%s\r\n", buf);
}

static void deinit(void* state) {
    assert(state);

    DebugState* s = (DebugState*)state;
    free(s);
}

Layer debug_layer_init(Renderer* r, Map* m, Vec2I* tile_pos) {
    DebugState* s = calloc(1, sizeof(*s));
    assert(s);

    s->r = r;
    s->m = m;
    s->tile_pos = tile_pos;

    return (Layer){
        .state = s,

        .handle_input = handle_input,
        .render = render,
        .deinit = deinit,
    };
}
