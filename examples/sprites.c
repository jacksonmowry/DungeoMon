#include "renderer.h"
#include "sx.h"
#include "tile.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc != 6) {
        fprintf(stderr,
                "usage: %s sprite_sheet.png tile_width tile_height "
                "horizontal_gap vertical_gap\n",
                argv[0]);
        return 1;
    }

    Vec2 tile_dims = VEC2(atoi(argv[2]), atoi(argv[3]));
    Vec2 tile_gaps = VEC2(atoi(argv[4]), atoi(argv[5]));
    Tilemap t = tilemap_load_png(argv[1], tile_dims, tile_gaps);

    Renderer r = sx_init(t.dimensions.x, t.dimensions.y, 4);

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
    r.draw_tile(r.state, VEC2(2, 2), VEC2(0, 0), t, person_nw, 0, 0);

    for (size_t col = 0; col < t.dimensions.x / t.tile_dimensions.x; col++) {
        Vec2 dest = VEC2(col, 0);
        if (col == 0) {
            r.draw_tile(r.state, dest, VEC2(0, 0), t, tl, 0, 0);
        } else if (col ==
                   (size_t)t.dimensions.x / (size_t)t.tile_dimensions.x - 1) {
            r.draw_tile(r.state, dest, VEC2(0, 0), t, tr, 0, 0);
        } else {
            r.draw_tile(r.state, dest, VEC2(0, 0), t, wall, 0, 0);
        }
    }

    printf("\n");

    r.render(r.state);

    printf("\n");
    r.cleanup(r.state);
    tilemap_deinit(t);
}
