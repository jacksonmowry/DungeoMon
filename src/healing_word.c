#include "healing_word.h"
#include "entity.h"

EntityUpdate healing_word() {
    return (EntityUpdate){
        .diff_health = HEALING_WORD_HEALING,
    };
}
