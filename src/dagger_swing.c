#include "dagger_swing.h"
#include "entity.h"

EntityUpdate dagger_swing() {
    return (EntityUpdate){
        .diff_health = DAGGER_DMG,
    };
}
