#pragma once

typedef struct Item {
    void* state;
    int value;
    void (*sell)(void* state);
} Item;
