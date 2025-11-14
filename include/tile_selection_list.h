#pragma once

#include "layer.h"
#include "map.h"
#include "renderer.h"
#include "vec.h"

Layer tile_selection_list_init(Renderer* r, Map* m, Vec2I* tile_pos);
