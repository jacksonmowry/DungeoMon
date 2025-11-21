#pragma once
#include "player.h"

typedef enum {
    FIREBALL,
    SWORD_SWING,
    ENERGY_BEAM,
    DAGGER_SWING,
    GREATSWORD_SWING,
    HEALING_WORD,
    PUNCH,
} PLAYER_MOVES;

#define MAX_PLAYER_MOVES 4

EntityUpdate player_move_lookup(struct Player* p, PLAYER_MOVES move);
