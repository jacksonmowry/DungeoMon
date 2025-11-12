#pragma once

typedef struct Vec2 {
    double x;
    double y;
} Vec2;

typedef struct Vec2I {
    int x;
    int y;
} Vec2I;

#define VEC2(a, b) ((Vec2){.x = (a), .y = (b)})
#define VEC2_SPLAT(a) ((Vec2){.x = (a), .y = (a)})
#define VEC2I_TO_VEC2(v) ((Vec2){.x = v.x, .y = v.y})

#define VEC2I(a, b) ((Vec2I){.x = (a), .y = (b)})
#define VEC2I_SPLAT(a) ((Vec2I){.x = (a), .y = (a)})
#define VEC2_TO_VEC2I(v) ((Vec2I){.x = v.x, .y = v.y})

/******************/
/* MULTIPLICATION */
/******************/
Vec2 vec2_mul_v(Vec2 x, Vec2 y);
Vec2 vec2_mul_a(Vec2 x, double a);
#define vec2_mul(x, y)                                                         \
    _Generic((y), Vec2: vec2_mul_v, default: vec2_mul_a)(x, y)

Vec2I vec2i_mul_v(Vec2I x, Vec2I y);
Vec2I vec2i_mul_a(Vec2I x, int a);
#define vec2i_mul(x, y)                                                        \
    _Generic((y), Vec2I: vec2i_mul_v, default: vec2i_mul_a)(x, y)

/************/
/* ADDITION */
/************/
Vec2 vec2_add_v(Vec2 x, Vec2 y);
Vec2 vec2_add_a(Vec2 x, double y);
#define vec2_add(x, y)                                                         \
    _Generic((y), Vec2: vec2_add_v, default: vec2_add_a)(x, y)

Vec2I vec2i_add_v(Vec2I x, Vec2I y);
Vec2I vec2i_add_a(Vec2I x, int y);
#define vec2i_add(x, y)                                                        \
    _Generic((y), Vec2I: vec2i_add_v, default: vec2i_add_a)(x, y)
