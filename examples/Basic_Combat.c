/*Compile Command:
//gcc -Iinclude -o tester \
    src/greatsword_swing.c \
    src/dagger_swing.c \
    src/healing_word.c \
    src/test.c \
    src/player.c \
    src/player_move_table.c \
    src/punch.c \
    src/sword_swing.c \
    src/energy_beam.c \
src/goblin.c \
src/fireball.c -fsanitize=address && ./tester


Look in include/player_move_table for each of the players moves.
0 - FIREBALL
1 - Sword Swing
2 - ENERGY_BEAM
3 - dagger_swing
4 - GREATSWORD_SWING
5 - HEALING_WORD
6 - Punch
Goblin moves: 0 - Stab Once  (10 dmg - player armor)
              1 - Stab Twice (20 dmg - playuer armor)
              2 - Loafing Around, does nothing.

*/

#include "entity.h"
#include "goblin.h"
#include "player.h"

int main() {

    Entity gob = goblin_init(10000, 10, 10, 10);
    Player p = player_init();
    while (gob.health != 0 && p.health != 0) {
        printf("Goblin Stats:\n_________\nhealth: %d\narmor: %d\ndamage: "
               "%d\n---------\n",
               gob.health, gob.armor, gob.damage);
        EntityUpdate update = gob.entity_vtable.produce_update(&gob);
        p.recieve_update(&p, update);
        printf("Player stats:\n_________\narmor: %d\nhealth: %d\nmana: "
               "%d\n---------\n",
               p.armor, p.health, p.mana);
        update = p.produce_update(&p);
        gob.entity_vtable.recieve_update(&gob, update);
    }
    if (gob.health == 0) {
        printf("gob died!!\n");
    }
    if (p.health == 0) {
        printf("player died!!\n");
    }
    return 0;
}
