#include "goblin.h"
#include "entity.h"
#include "id.h"
#include "player.h"
#include <stdio.h>
#include <stdlib.h>
typedef struct GoblinState {
    void* moves;
    char* message;
} GoblinState;

static void recieve_update(Entity* this, EntityUpdate update) {

    this->health += update.diff_health;
    this->armor += update.diff_armor;
    this->mana += update.diff_mana;
    return;
}
static EntityUpdate produce_update(void* state, Entity* this, int move) {
    GoblinState* s = (GoblinState*)state;
    EntityUpdate update;
    switch (move) {

    case (1):
        printf("Goblin Chose to Stab.\n");
        update.diff_health = this->damage;
        break;
    case (2):
        printf("Goblin Chose to Stab TWICE\n");
        update.diff_health = (this->damage * 2);
        break;
    default:
        printf("Goblin Chose to do nothing.");
    }
    return update;
}

static void interact(Entity* ent, Player* player) {
    GoblinState* s = (GoblinState*)ent->state;
    int move;
    printf("Select Goblin Move:(1-4)");
    scanf("%d", &move);
    EntityUpdate info = produce_update(ent->state, ent, move);
}
static void speak(void* state, const char* statement) {
    GoblinState* s = (GoblinState*)state;
    printf("%s", s->message);
    return;
}

static void deinit(Entity* ent) { free(ent->state); }

Entity goblin_init(double health, double damage, double armor, double mana) {
    GoblinState* gob = calloc(1, sizeof(GoblinState));
    *gob = (GoblinState){
        .message = "I be goblin em",
    };

    return (Entity){
        .state = gob,
        .id = GOBLIN_ID,
        .interact = interact,
        .speak = speak,
        .deinit = deinit,
        .damage = damage,
        .health = health,
        .armor = armor,
        .produce_update = produce_update,
        .recieve_update = recieve_update,
    };
}
