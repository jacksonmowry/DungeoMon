#include "goblin.h"
#include "entity.h"
#include "id.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_MOVES 3

typedef enum {
    STAB,
    DOUBLE_STAB,
    NOTHING,
} GOBLIN_MOVES;

typedef struct GoblinState {
    void* moves;
    char* message;
} GoblinState;

static void move(Entity* this, Vec2 velo, int x_pos, int y_pos) {
    assert(this);

    printf("called move\n");
    return;
}

static void recieve_update(Entity* this, EntityUpdate update) {
    assert(this);

    this->health += update.diff_health;
    this->armor += update.diff_armor;
    this->mana += update.diff_mana;
    return;
}
static EntityUpdate produce_update(Entity* this) {
    assert(this && this->state);

    GoblinState* s = (GoblinState*)this->state;
    EntityUpdate update = {
        .diff_health = 0,
        .diff_mana = 0,
        .diff_armor = 0,
    };

    GOBLIN_MOVES move;
    printf("Select Goblin Move:(0-3)\n");
    scanf("%d", &move);
    switch (move) {

    case STAB:
        printf("Goblin Chose to Stab.\n");
        update.diff_health = -this->damage;
        break;
    case DOUBLE_STAB:
        printf("Goblin Chose to Stab TWICE\n");
        update.diff_health = -(this->damage * 2);
        break;
    case NOTHING:
        printf("The goblin was loafing around..\n");
        break;
    default:
        printf("Goblin Chose to do nothing.");
    }
    return update;
}

static void speak(Entity* this, const char* statement) {
    assert(this && this->state);

    GoblinState* s = (GoblinState*)this->state;
    printf("%s", s->message);
    return;
}

static void deinit(Entity* ent) {
    assert(ent && ent->state);
    free(ent->state);
}

// NOTE: Create move funcitonality here later.
const static struct entity_vtable goblin_vtable =
    (struct entity_vtable){.move = move,
                           .speak = speak,
                           .produce_update = produce_update,
                           .recieve_update = recieve_update,
                           .deinit = deinit};

Entity goblin_init(double health, double damage, double armor, double mana) {
    GoblinState* gob = calloc(1, sizeof(GoblinState));
    assert(gob);

    *gob = (GoblinState){
        .message = "I be goblin em",
    };

    return (Entity){
        .state = gob,
        .id = GOBLIN_ID,
        .damage = damage,
        .health = health,
        .armor = armor,
        .entity_vtable = goblin_vtable,
    };
}
