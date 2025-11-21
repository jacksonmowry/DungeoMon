#include "energy_beam.h"
#include "entity.h"

EntityUpdate energy_beam() {
    return (EntityUpdate){
        .diff_health = -ENERGY_BEAM_DMG,
    };
}
