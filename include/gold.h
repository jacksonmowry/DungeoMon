#include <item.h>
#include <stdlib.h>

typedef struct Gold {
    void* state;
    Item info;
} Gold;

Gold* gold_init(int quantity);
