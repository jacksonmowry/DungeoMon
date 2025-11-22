#include "player.h"
#include "damage_types.h"
#include "entity.h"
#include "player_move_table.h"
#include "spell.h"
#include "weapon.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
// WARNING: Not sure what needs to be in the player state

#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)
#define STARTING_PLAYER_HEALTH 100
#define STARTING_PLAYER_MANA 100
#define STARTING_PLAYER_ARMOR 5
#define STARTING_PLAYER_SPELLS 5
#define STARTING_PLAYER_WEAPONS 5

static Damages WeaponLookup(int id) { return (Damages){}; }

void move(Player* p, Vec2 velocity, int x_pos, int y_pos) {
    fprintf(stderr, "Called Move\n");
    return;
}
void speak(Player* p, const char* statement) {
    assert(statement);
    printf("%s\n", statement);
    return;
}

EntityUpdate produce_update(Player* p) {
    PLAYER_MOVES move;
    printf("Select Player Move.\n");
    scanf("%d", &move);
    return player_move_lookup(p, move);
}

static void recieve_update(Player* player, EntityUpdate player_update) {
    if (abs(player_update.diff_health) > player->armor) {
        player->health += (player_update.diff_health + player->armor);
    }
    player->mana += player_update.diff_mana;
    player->armor += player_update.diff_armor;
    // player->num_spells += player_update.diff_num_spells;
    // player->num_weapons += player_update.diff_num_weapons;
    return;
}
static void player_death_sequence() {}

// NOTE: Layout for more generalized attacks.
static EntityUpdate PlayerAttack(const Player* player, const Entity enemy,
                                 int weapon_id, int spell_id) {
    EntityUpdate update = {0};
    if (weapon_id != -1) {
        const Damages types = WeaponLookup(weapon_id);
        update.diff_health += MAX((-1) * (types.physical_dmg - enemy.armor), 0);
        update.diff_health += MAX((-1) * (types.magic_dmg - enemy.armor), 0);
    }
    if (spell_id != -1) {
        const Damages types = WeaponLookup(weapon_id);
        update.diff_health += MAX((-1) * (types.magic_dmg - enemy.armor), 0);
        update.diff_health += MAX((-1) * (types.physical_dmg - enemy.armor), 0);
    }

    return update;
}

Player player_init(void) {
    return (Player){
        .armor = STARTING_PLAYER_ARMOR,
        .health = STARTING_PLAYER_HEALTH,
        .mana = STARTING_PLAYER_MANA,
        .spells = calloc(5, sizeof(Spell)),
        .weapons = calloc(5, sizeof(Weapon)),
        .produce_update = produce_update,
        .recieve_update = recieve_update,
        .move = move,
        .speak = speak,
    };
}
// NOTE: Keep things as value not reference when possible, const things you
// arent changing. Forward thinking (multi damage types).
