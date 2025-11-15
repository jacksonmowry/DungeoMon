#pragma once

#include "layer.h"
#include "map.h"
#include "renderer.h"
#include "vec.h"

Layer debug_layer_init(Renderer* r, Map* m, Vec2I* tile_pos);
