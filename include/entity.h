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
    bool animation;
    int x_pos;
    int y_pos;
    int hp;
    int mana;
    int armor;
    bool death;
    int base_dmg;
    void (*move)(void* state, Vec2 velocity, int x_pos, int y_pos);
    void (*battle)(void* state);
    void (*speak)(void* state, const char* statement);
    void (*update)(struct Entity* this, EntityUpdate update);
} Entity;
