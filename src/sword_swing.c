#include "sword_swing.h"
#include "entity.h"

EntityUpdate sword_swing() {
    return (EntityUpdate){
        .diff_health = -SWORD_DMG,
        .diff_armor = -SWORD_ARMOR_DIFF,
    };
}
