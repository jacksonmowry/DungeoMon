#include "player.h"
#include "damage_types.h"
#include "entity.h"
#include "spell.h"
#include "weapon.h"
#include <stdbool.h>
#include <stdlib.h>
// WARNING: Not sure what needs to be in the player state

#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)
#define STARTING_PLAYER_HEALTH 100
#define STARTING_PLAYER_MANA 50
#define STARTING_PLAYER_ARMOR 5
#define STARTING_PLAYER_SPELLS 5
#define STARTING_PLAYER_WEAPONS 5

static Damages WeaponLookup(int id) { return (Damages){}; }
static Player player_init(void) {
    Player empty_player;
    empty_player.armor = STARTING_PLAYER_ARMOR;
    empty_player.health = STARTING_PLAYER_HEALTH;
    empty_player.mana = STARTING_PLAYER_MANA;
    empty_player.spells = calloc(5, sizeof(Spell));
    empty_player.weapons = calloc(5, sizeof(Weapon));
    return empty_player;
}

static void player_status_change(Player* player_state,
                                 const EntityUpdate player_update) {
    player_state->health += player_update.diff_health;
    player_state->mana += player_update.diff_mana;
    player_state->armor += player_update.diff_armor;
    player_state->num_spells += player_update.diff_num_spells;
    player_state->num_weapons += player_update.diff_num_weapons;
    return;
}
static void player_death_sequence() {}

// NOTE: Layout for more generalized attacks.
static EntityUpdate PlayerAttack(Player* player, const Entity enemy,
                                 int weapon_id, int spell_id) {
    EntityUpdate update = {0};
    if (weapon_id != -1) {
        Damages types = WeaponLookup(weapon_id);
        update.diff_health += MAX((-1) * (types.physical_dmg - enemy.armor), 0);
        update.diff_health += MAX((-1) * (types.magic_dmg - enemy.armor), 0);
    }
    if (spell_id != -1) {
        Damages types = WeaponLookup(weapon_id);
        update.diff_health += MAX((-1) * (types.magic_dmg - enemy.armor), 0);
        update.diff_health += MAX((-1) * (types.physical_dmg - enemy.armor), 0);
    }

    return update;
}

// NOTE: Keep things as value not reference when possible, const things you
// arent changing. Forward thinking (multi damage types).
