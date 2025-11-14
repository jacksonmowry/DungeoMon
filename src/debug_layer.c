#include "layer.h"
#include "map.h"

typedef struct DebugState {
    Map* m;
    Vec2I* tile_pos;
    int* attribute_overlay;

    bool debug;
} DebugState;

LayerEventResponse handle_input(void* state, Event e) {
    // Handle
    switch (e.event_type) {

    }

    return (LayerEventResponse){0};
}

void render(void* state) {
    DebugState* s = (DebugState*)state;

    printf("+---------------------------------------+\r\n");
    printf("|'q'      | to quit                     |\r\n");
    printf("|'hjkl'   | to move left down up right  |\r\n");
    printf("|'r'      | to rotate tile clockwise    |\r\n");
    printf("|'v'      | to flip tile vertically     |\r\n");
    printf("|'r'      | to flip tile horizontally   |\r\n");
    printf("|'Enter'  | to change a tile            |\r\n");
    printf("|'Escape' | to unselect a tile          |\r\n");
    printf("|'d'      | to enable debug view        |\r\n");
    printf("+---------------------------------------+\r\n");
    static char buf[4096];
    printf("\033[0J");
    printf("Overlay highlight: ");
    switch (*s->attribute_overlay) {
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
    map_tile_attributes_debug(*s->m, *s->tile_pos, buf, 4096);
    printf("%s\r\n", buf);
}

Layer debug_layer_init(Map* m, Vec2I* tile_pos, int* attribute_overlay) {
    DebugState* s = calloc(1, sizeof(*s));
    s->m = m;
    s->tile_pos = tile_pos;
    s->attribute_overlay = attribute_overlay;

    return (Layer){
        .state = s,

        .handle_input = handle_input,
        .render = render,
    };
}
