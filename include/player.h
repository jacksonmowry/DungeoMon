#pragma once

#include <entity.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vec.h>
typedef struct Player {

    int health;
    int armor;
    int mana;
    int damage;
    void* spells;
    void* weapons;
    int num_spells;
    int num_weapons;
    void (*move)(struct Player* player, Vec2 velocity, int x_pos, int y_pos);
    void (*speak)(struct Player* player, const char* statement);
    EntityUpdate (*produce_update)(struct Player* p);
    void (*recieve_update)(struct Player* p, const EntityUpdate update);
    void (*deinit)(struct Entity* this);
} Player;

Player player_init();
