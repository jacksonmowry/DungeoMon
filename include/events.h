#pragma once

#include "queue.h"

typedef struct Event {
    enum {
        NOP,
        UP,
        DOWN,
        LEFT,
        RIGHT,
        R,
        V,
        B,
        D,
        T,
        W,
        ENTER,
        ESCAPE,
        QUIT,
        POPPED
    } event_type;
} Event;

LockableQueuePrototypes(Event);

typedef LockableQueue_Event EventQueue;
typedef LockableQueue_Event_Result EventQueueResult;
