#include "punch.h"
#include "entity.h"

EntityUpdate punch() {
    return (EntityUpdate){
        .diff_health = -PUNCH_DMG,
    };
}
