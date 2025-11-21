#include "player_move_table.h"
#include "dagger_swing.h"
#include "energy_beam.h"
#include "entity.h"
#include "fireball.h"
#include "greatsword_swing.h"
#include "healing_word.h"
#include "player.h"
#include "punch.h"
#include "sword_swing.h"

EntityUpdate player_move_lookup(Player* p, PLAYER_MOVES move) {

    switch (move) {
    case (FIREBALL):
        if (p->mana < FIREBALL_MANA_COST) {
            fprintf(stderr, "Did not have enough mana to cast FIREBALL\n");
            return (EntityUpdate){0};
        } else {
            p->mana -= FIREBALL_MANA_COST;
            return fireball();
        }
    case (SWORD_SWING):
        return sword_swing();
    case (GREATSWORD_SWING):
        return greatsword_swing();
    case (DAGGER_SWING):
        return dagger_swing();
    case (HEALING_WORD):

        if (p->mana < HEALING_WORD_MANA_COST) {
            fprintf(stderr, "Did not have enough mana to cast HEALING_WORD\n");
        } else {
            p->mana -= HEALING_WORD_MANA_COST;
            p->health += HEALING_WORD_HEALING;
        }
        return (EntityUpdate){0};
    case (PUNCH):
        return punch();
    case (ENERGY_BEAM):
        if (p->mana < HEALING_WORD_MANA_COST) {
            fprintf(stderr, "Did not have enough mana to cast HEALING_WORD\n");
            return (EntityUpdate){0};
        } else {
            p->mana -= ENERGY_BEAM_MANA_COST;
            return energy_beam();
        }
    }
}
