#include "layer.h"
#include "map.h"
#include "renderer.h"
#include "tile_selection_list.h"

typedef struct MapState {
    Map* m;      // Not owned
    Renderer* r; // Not owned

    Vec2I tile_pos;
    bool debug;
    int attribute_overlay;

    Layer* l;
} MapState;

static LayerEventResponse handle_input(void* state, Event e) {
    MapState* s = (MapState*)state;
    LayerEventResponse response = {0};

    size_t index = (s->tile_pos.y * s->m->dimensions.x) + s->tile_pos.x;
    switch (e.event_type) {
    case UP:
        s->tile_pos.y = s->tile_pos.y != 0 ? s->tile_pos.y - 1 : s->tile_pos.y;
        response.status = HANLDED;
        break;
    case DOWN:
        s->tile_pos.y = s->tile_pos.y != 19 ? s->tile_pos.y + 1 : s->tile_pos.y;
        response.status = HANLDED;
        break;
    case LEFT:
        s->tile_pos.x = s->tile_pos.x != 0 ? s->tile_pos.x - 1 : s->tile_pos.x;
        response.status = HANLDED;
        break;
    case RIGHT:
        s->tile_pos.x = s->tile_pos.x != 29 ? s->tile_pos.x + 1 : s->tile_pos.x;
        response.status = HANLDED;
        break;
    case R:
        s->m->tile_rotations[index] = (s->m->tile_rotations[index] + 1) % 4;
        response.status = HANLDED;
        break;
    case V:
        s->m->tile_attributes[index] ^= TILE_VERTICAL_FLIP;
        response.status = HANLDED;
        break;
    case B:
        s->m->tile_attributes[index] ^= TILE_HORIZONTAL_FLIP;
        response.status = HANLDED;
        break;
    case D:
        s->debug = !s->debug;
        printf("\033[2J");
        response.status = HANLDED;
        break;
    case T:
        if (s->debug) {
            s->attribute_overlay =
                (s->attribute_overlay + 1) % TILE_NUM_ATTRIBUTES;
        }
        response.status = HANLDED;
        break;
    case W:
        s->m->tile_attributes[index] ^= TILE_WALL;
        response.status = HANLDED;
        break;
    case ENTER:
        s->l = calloc(1, sizeof(*s->l));
        *s->l = tile_selection_list_init(s->r, s->m, &s->tile_pos);

        response.status = PUSH;
        response.l = s->l;
        break;
    case QUIT:
        response.status = POP;
        break;
    case POPPED:
        if (s->l) {
            free(s->l);
        }
        s->l = NULL;
        response.status = HANLDED;
        break;
    default:
        break;
    }

    return response;
}

static void render(void* state) {
    MapState* s = (MapState*)state;

    s->r->draw_map(s->r->state, *s->m);

    if (s->attribute_overlay != TILE_NORMAL && s->debug) {
        // Dsw a blue tint over all tiles with the currently selected
        // attribute
        for (size_t row = 0; row < s->m->dimensions.y; row++) {
            for (size_t col = 0; col < s->m->dimensions.x; col++) {
                size_t index = (row * s->m->dimensions.x) + col;
                if (s->m->tile_attributes[index] &
                    (1 << (s->attribute_overlay - 1))) {
                    Vec2I pos = VEC2I(col, row);
                    s->r->draw_rect_filled(
                        s->r->state,
                        tile_coords(pos, s->m->t.tile_dimensions.x, NW),
                        tile_coords(pos, s->m->t.tile_dimensions.x, SE),
                        (RGBA){.r = 0x00, .b = 0xFF, .g = 0x00, .a = 0x45},
                        (RGBA){.r = 0x00, .b = 0xFF, .g = 0x00, .a = 0x45});
                }
            }
        }
    }

    // Dsw red outline
    s->r->draw_rect(s->r->state,
                    tile_coords(s->tile_pos, s->m->t.tile_dimensions.x, NW),
                    tile_coords(s->tile_pos, s->m->t.tile_dimensions.x, SE),
                    (RGBA){.r = 0xFF, .a = 0x95});

    return;
}

Layer map_layer_init(Renderer* r, Map* m) {
    MapState* s = calloc(1, sizeof(*s));

    s->m = m;
    s->r = r;

    return (Layer){
        .state = s,

        .handle_input = handle_input,
        .render = render,
    };
}
