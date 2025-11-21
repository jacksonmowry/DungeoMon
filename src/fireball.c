#include "entity.h"
#include <fireball.h>

EntityUpdate fireball(void) {
    return (EntityUpdate){
        .diff_armor = 0,
        .diff_health = FIREBALL_DMG,
        .diff_mana = 0,
    };
}
