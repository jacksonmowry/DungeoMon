#include "tile_selection_list.h"
#include "layer.h"
#include "map.h"
#include "renderer.h"
#include "tile.h"
#include "vec.h"
#include <assert.h>
#include <stdbool.h>

// Anything that is a pointer is data we do not own
typedef struct TileListState {
    Renderer* r;
    Map* m;
    Vec2I* tile_pos;

    size_t list_scroll_pos;
    Vec2I cursor_pos;

    size_t tiles_per_row;
    size_t rows_of_tiles;
    size_t num_rendered_rows;
} TileListState;

static LayerEventResponse handle_input(void* state, Event e) {
    assert(state);

    TileListState* s = (TileListState*)state;
    LayerEventResponse response = {0};

    switch (e.event_type) {
    case UP: {
        // Tile selection list
        if (s->cursor_pos.y != 0) {
            // Move normally, we're not at the top
            s->cursor_pos.y -= 1;
        } else if (s->cursor_pos.y == 0 && s->list_scroll_pos != 0) {
            // We're at the bottom of the screen, scroll down
            s->list_scroll_pos -= 1;
        }
        response.status = HANDLED;
    } break;
    case DOWN: {
        if (s->cursor_pos.y != 6) {
            // Move normally, we're not at the bottom
            s->cursor_pos.y += 1;
        } else if (s->cursor_pos.y == 6) {
            // We're at the bottom of the screen, scroll down
            s->list_scroll_pos += 1;
        }
        response.status = HANDLED;
    } break;
    case LEFT:
        s->cursor_pos.x =
            s->cursor_pos.x != 0 ? s->cursor_pos.x - 1 : s->cursor_pos.x;
        response.status = HANDLED;
        break;
    case RIGHT:
        s->cursor_pos.x =
            s->cursor_pos.x != 11 ? s->cursor_pos.x + 1 : s->cursor_pos.x;
        response.status = HANDLED;
        break;
    case ENTER: {
        // Tile select screen is up, we need to now replace the tile on
        // the map with the currently selected tile
        const int starting_tile_index = s->list_scroll_pos * s->tiles_per_row;
        Vec2I tile =
            VEC2I(starting_tile_index % (s->m->t->dimensions_in_tiles.x),
                  (starting_tile_index / (s->m->t->dimensions_in_tiles.x)));

        const int tile_offset =
            (s->cursor_pos.y * s->tiles_per_row) + s->cursor_pos.x;
        tile = vec2i_add(tile,
                         VEC2I((tile_offset % s->m->t->dimensions_in_tiles.x),
                               (tile_offset / s->m->t->dimensions_in_tiles.x)));

        s->m->tiles[(s->tile_pos->y * s->m->dimensions.x) + s->tile_pos->x] =
            tile;
        response.status = POP;
    } break;
    case ESCAPE:
        response.status = POP;
        break;
    default:
        response.status = IGNORED;
    }

    return response;
}

static void render(void* state) {
    assert(state);

    TileListState* s = (TileListState*)state;

    s->r->draw_rect_filled(
        s->r->state, tile_coords(VEC2I(2, 2), s->m->t->tile_dimensions.x, NW),
        tile_coords(VEC2I(27, 17), s->m->t->tile_dimensions.x, SE), WHITE,
        (RGBA){.r = 0xAF, .g = 0xAF, .b = 0xAF, .a = 0xAF});

    // First check if they attempted to scroll past the end
    if (s->list_scroll_pos >= s->rows_of_tiles - (s->num_rendered_rows - 1)) {
        s->list_scroll_pos = s->rows_of_tiles - (s->num_rendered_rows - 1);
    }

    // When the user does scroll down the last row of tiles may be
    // incomplete, check this and move the the last tile if so
    if (s->list_scroll_pos == s->rows_of_tiles - (s->num_rendered_rows - 1) &&
        (size_t)s->cursor_pos.y == s->num_rendered_rows - 1) {
        // Check how many tiles are being rendered in the last row
        const int last_row_tiles = s->m->t->num_tiles % s->tiles_per_row;

        if (s->cursor_pos.x >= last_row_tiles) {
            s->cursor_pos.x = last_row_tiles - 1;
        }
    }

    // Now dsw all of the tiles that will fit
    const int starting_tile_index = s->list_scroll_pos * s->tiles_per_row;
    Vec2I tile =
        VEC2I(starting_tile_index % (s->m->t->dimensions_in_tiles.x),
              (starting_tile_index / (s->m->t->dimensions_in_tiles.x)));

    for (size_t row = 3; row < 17; row += 2) {
        for (size_t col = 3; col < 27; col += 2) {
            // Don't dsw past the end of the tilemap
            if ((tile.y *
                 (s->m->t->dimensions.x / s->m->t->tile_dimensions.x)) +
                    (size_t)tile.x >=
                s->m->t->num_tiles) {
                break;
            }

            s->r->draw_tile(s->r->state, VEC2I(col, row), VEC2I(4, 4), s->m->t,
                            tile, 0, 0);

            tile.x++;
            if (tile.x == s->m->t->dimensions.x / s->m->t->tile_dimensions.x) {
                tile.x = 0;
                tile.y++;
            }
        }
    }

    // Dsw tile picker
    const Vec2I picker_dsw_nw = vec2i_add(
        tile_coords(vec2i_add(vec2i_mul(s->cursor_pos, 2), VEC2I(3, 3)),
                    s->m->t->tile_dimensions.x, NW),
        3);
    const Vec2I picker_dsw_se = vec2i_add(
        tile_coords(vec2i_add(vec2i_mul(s->cursor_pos, 2), VEC2I(3, 3)),
                    s->m->t->tile_dimensions.x, SE),
        5);
    s->r->draw_rect(s->r->state, picker_dsw_nw, picker_dsw_se,
                    (RGBA){.r = 0xFF, .g = 0xFF, .b = 0x00, .a = 0x95});

    return;
}

static void deinit(void* state) {
    assert(state);

    TileListState* s = (TileListState*)state;
    free(s);
}

Layer tile_selection_list_init(Renderer* r, Map* m, Vec2I* tile_pos) {
    TileListState* s = calloc(1, sizeof(*s));

    s->r = r;
    s->m = m;
    s->tile_pos = tile_pos;

    s->tiles_per_row = ((27 - 3) / 2);
    s->rows_of_tiles = m->t->num_tiles / s->tiles_per_row;
    s->num_rendered_rows = (17 - 3) / 2;

    return (Layer){
        .state = s,

        .handle_input = handle_input,
        .render = render,
        .deinit = deinit,
    };
}
