#include "debug_layer.h"
#include "layer.h"
#include "map.h"
#include "renderer.h"
#include "tile_selection_list.h"
#include <assert.h>

typedef struct MapState {
    Map* m;      // Not owned
    Renderer* r; // Not owned

    Vec2I tile_pos;
    bool debug;
    int attribute_overlay;

    Layer normal_layer;
#ifdef DEBUG
    Layer debug_layer;
#endif
} MapState;

static LayerEventResponse handle_input(void* state, Event e) {
    MapState* s = (MapState*)state;
    LayerEventResponse response = {0};

    size_t index = (s->tile_pos.y * s->m->dimensions.x) + s->tile_pos.x;
    switch (e.event_type) {
    case UP:
        s->tile_pos.y = s->tile_pos.y != 0 ? s->tile_pos.y - 1 : s->tile_pos.y;
        response.status = HANDLED;
        break;
    case DOWN:
        s->tile_pos.y = s->tile_pos.y != 19 ? s->tile_pos.y + 1 : s->tile_pos.y;
        response.status = HANDLED;
        break;
    case LEFT:
        s->tile_pos.x = s->tile_pos.x != 0 ? s->tile_pos.x - 1 : s->tile_pos.x;
        response.status = HANDLED;
        break;
    case RIGHT:
        s->tile_pos.x = s->tile_pos.x != 29 ? s->tile_pos.x + 1 : s->tile_pos.x;
        response.status = HANDLED;
        break;
    case R:
        s->m->tile_rotations[index] = (s->m->tile_rotations[index] + 1) % 4;
        response.status = HANDLED;
        break;
    case V:
        s->m->tile_attributes[index] ^= TILE_VERTICAL_FLIP;
        response.status = HANDLED;
        break;
    case B:
        s->m->tile_attributes[index] ^= TILE_HORIZONTAL_FLIP;
        response.status = HANDLED;
        break;
#ifdef DEBUG
    case D:
        printf("\033[2J");
        if (s->debug) {
            response.status = DEBUG_POP;
        } else {
            s->debug_layer = debug_layer_init(s->r, s->m, &s->tile_pos);

            response.status = DEBUG_PUSH;
            response.l = &s->debug_layer;
        }
        s->debug = !s->debug;
        break;
#endif
    case W:
        s->m->tile_attributes[index] ^= TILE_WALL;
        response.status = HANDLED;
        break;
    case ENTER:
        s->normal_layer = tile_selection_list_init(s->r, s->m, &s->tile_pos);

        response.status = PUSH;
        response.l = &s->normal_layer;
        break;
    case POPPED:
        s->normal_layer = (Layer){0};
        response.status = HANDLED;
        break;
    default:
        response.status = IGNORED;
        break;
    }

    return response;
}

static void render(void* state) {
    MapState* s = (MapState*)state;

    s->r->draw_map(s->r->state, *s->m);

    // Dsw red outline
    s->r->draw_rect(s->r->state,
                    tile_coords(s->tile_pos, s->m->t.tile_dimensions.x, NW),
                    tile_coords(s->tile_pos, s->m->t.tile_dimensions.x, SE),
                    (RGBA){.r = 0xFF, .a = 0x95});

    return;
}

static void deinit(void* state) {
    MapState* s = (MapState*)state;
    free(s);
}

Layer map_layer_init(Renderer* r, Map* m) {
    MapState* s = calloc(1, sizeof(*s));

    s->m = m;
    s->r = r;

    return (Layer){
        .state = s,

        .handle_input = handle_input,
        .render = render,
        .deinit = deinit,
    };
}
