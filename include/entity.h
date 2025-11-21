#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <vec.h>

typedef struct EntityUpdate {

    int diff_health;
    int diff_armor;
    int diff_mana;
    void* diff_spells;
    void* diff_weapons;
    int diff_num_spells;
    int diff_num_weapons;

} EntityUpdate;
typedef struct Entity {
    void* state;
    int id;
    bool animation;
    int x_pos;
    int y_pos;
    int health;
    int damage;
    int mana;
    int armor;
    bool death;
    int base_dmg;
    const struct entity_vtable {

        void (*move)(struct Entity* this, Vec2 velocity, int x_pos, int y_pos);
        void (*speak)(struct Entity* this, const char* statement);
        EntityUpdate (*produce_update)(struct Entity* this);
        void (*recieve_update)(struct Entity* this, EntityUpdate update);
        void (*deinit)(struct Entity* this);

    } entity_vtable;
} Entity;
