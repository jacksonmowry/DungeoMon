#pragma once

#include "events.h"
#include <stdbool.h>

typedef struct Layer Layer;

typedef enum LayerEventStatus {
    IGNORED,
    HANLDED,
    POP,
    PUSH,
} LayerEventStatus;

typedef struct LayerEventResponse {
    LayerEventStatus status;
    Layer* l;
} LayerEventResponse;

typedef struct Layer {
    void* state;

    bool exclusive_mode;
    bool dont_render;

    LayerEventResponse (*handle_input)(void* state, Event e);
    void (*render)(void* state);
} Layer;
