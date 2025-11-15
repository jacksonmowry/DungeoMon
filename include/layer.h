#pragma once

#include "events.h"
#include <stdbool.h>

typedef struct Layer Layer;

typedef enum LayerEventStatus {
    IGNORED,
    HANDLED,
    POP,
    PUSH,
#ifdef DEBUG
    DEBUG_PUSH,
    DEBUG_POP,
#endif
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
    void (*deinit)(void* state);
} Layer;
