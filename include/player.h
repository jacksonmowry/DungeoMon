#pragma once

typedef struct Player {
    int health;
    int armor;
    int mana;
    int dmg;
    void* spells;
    void* weapons;
    int num_spells;
    int num_weapons;
} Player;

Player player_init();
