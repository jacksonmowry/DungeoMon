#pragma once

typedef struct Vec2 {
    double x;
    double y;
} Vec2;

#define VEC2(a, b) ((Vec2){.x = (a), .y = (b)})
#define VEC2_SPLAT(a) ((Vec2){.x = (a), .y = (a)})

/******************/
/* MULTIPLICATION */
/******************/
Vec2 vec2_mul_v(Vec2 x, Vec2 y);
Vec2 vec2_mul_a(Vec2 x, double a);
#define vec2_mul(x, y)                                                         \
    _Generic((y), Vec2: vec2_mul_v, default: vec2_mul_a)(x, y)

/************/
/* ADDITION */
/************/
Vec2 vec2_add_v(Vec2 x, Vec2 y);
Vec2 vec2_add_a(Vec2 x, double y);
#define vec2_add(x, y)                                                         \
    _Generic((y), Vec2: vec2_add_v, default: vec2_add_a)(x, y)
