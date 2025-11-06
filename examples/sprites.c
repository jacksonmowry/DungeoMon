#include "renderer.h"
#include "sx.h"
#include "tile.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

    Renderer r = sx_init(t.dimensions.x, t.dimensions.y, 6);

    for (size_t i = 0; i < t.dimensions.y; i++) {
        for (size_t j = 0; j < t.dimensions.x; j++) {
            RGBA p = t.pixels[(i * (size_t)t.dimensions.x) + j];
            r.draw_pixel(r.state, VEC2(j, i),
                         t.pixels[(i * (size_t)t.dimensions.x) + j]);
        }
    }

    r.render(r.state);

    Vec2 person_nw = VEC2(4, 0);
    Vec2 tl = VEC2(0, 0);
    Vec2 wall = VEC2(1, 0);
    Vec2 tr = VEC2(3, 0);
    Vec2 bl = VEC2(0, 2);
    Vec2 br = VEC2(3, 2);
    r.draw_tile(r.state, VEC2(2, 2), VEC2(0, 0), t, person_nw, 0, 0);

    Vec2 bottom_walls[4] = {VEC2(2, 4), VEC2(3, 4), VEC2(2, 5), VEC2(3, 5)};

    for (size_t col = 0; col < t.dimensions.x / t.tile_dimensions.x; col++) {
        Vec2 dest_top = VEC2(col, 0);
        Vec2 dest_bot = VEC2(col, t.dimensions.y / t.tile_dimensions.y - 1);
        if (col == 0) {
            r.draw_tile(r.state, dest_top, VEC2(0, 0), t, tl, 0, 0);
            r.draw_tile(r.state, dest_bot, VEC2(0, 0), t, bl, 0, 0);
        } else if (col ==
                   (size_t)t.dimensions.x / (size_t)t.tile_dimensions.x - 1) {
            r.draw_tile(r.state, dest_top, VEC2(0, 0), t, tr, 0, 0);
            r.draw_tile(r.state, dest_bot, VEC2(0, 0), t, br, 0, 0);
        } else {
            r.draw_tile(r.state, dest_top, VEC2(0, 0), t, wall, 0, 0);

            int rn = rand() % 4;
            r.draw_tile(r.state, dest_bot, VEC2(0, 0), t, bottom_walls[rn], 0,
                        0);
        }
    }

    printf("\n\n");

    Vec2 left_wall = VEC2(0, 1);
    Vec2 right_wall = VEC2(3, 1);
    for (size_t row = 1; row < t.dimensions.y / t.tile_dimensions.y - 1;
         row++) {
        r.draw_tile(r.state, VEC2(0, row), VEC2(0, 0), t, left_wall, 0, 0);
        r.draw_tile(
            r.state,
            VEC2((size_t)(t.dimensions.x / t.tile_dimensions.x) - 1, row),
            VEC2(0, 0), t, right_wall, 0, 0);
    }

    Vec2 fire = VEC2(8, 8);
    size_t width_in_tiles = t.dimensions.x / t.tile_dimensions.x;
    size_t height_in_tiles = t.dimensions.y / t.tile_dimensions.y;
    for (int i = 0; i < 3; i++) {
        Vec2 pos = VEC2(rand() % (width_in_tiles - 1) + 1,
                        rand() % (height_in_tiles - 2) + 1);

        r.draw_tile(r.state, pos, VEC2(0, 0), t, fire, 0, 0);
    }

    r.render(r.state);

    printf("\n");
    r.cleanup(r.state);
    tilemap_deinit(t);
}
