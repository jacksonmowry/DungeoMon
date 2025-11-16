#include <gold.h>
#include <item.h>

Gold* gold_init(int quantity) {
    Gold* gold;
    gold->info.value = 1;
    gold->info.quantity = quantity;
    gold->state = NULL;
    return gold;
}

void* add_item(void* state) {

};
