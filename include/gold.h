#pragma once

#include <item.h>

typedef struct Gold {
    void* state;
    Item info;
} Gold;

Gold* gold_init(int quantity);
