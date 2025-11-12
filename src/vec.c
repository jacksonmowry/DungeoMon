#include "vec.h"

Vec2 vec2_mul_v(Vec2 x, Vec2 y) {
    return (Vec2){.x = x.x * y.x, .y = x.y * y.y};
}

Vec2 vec2_mul_a(Vec2 x, double a) { return (Vec2){.x = x.x * a, .y = x.y * a}; }

Vec2 vec2_add_v(Vec2 x, Vec2 y) {
    return (Vec2){.x = x.x + y.x, .y = x.y + y.y};
}

Vec2 vec2_add_a(Vec2 x, double a) { return (Vec2){.x = x.x + a, .y = x.y + a}; }

Vec2I vec2i_mul_v(Vec2I x, Vec2I y) {
    return (Vec2I){.x = x.x * y.x, .y = x.y * y.y};
}

Vec2I vec2i_mul_a(Vec2I x, int a) {
    return (Vec2I){.x = x.x * a, .y = x.y * a};
}

Vec2I vec2i_add_v(Vec2I x, Vec2I y) {
    return (Vec2I){.x = x.x + y.x, .y = x.y + y.y};
}

Vec2I vec2i_add_a(Vec2I x, int a) {
    return (Vec2I){.x = x.x + a, .y = x.y + a};
}
