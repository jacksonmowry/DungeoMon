#include "greatsword_swing.h"
#include "entity.h"

EntityUpdate greatsword_swing() {
    return (EntityUpdate){
        .diff_health = -GREATSWORD_DMG,
        .diff_armor = -GREATSWORD_ARMOR_DIFF,
    };
}
