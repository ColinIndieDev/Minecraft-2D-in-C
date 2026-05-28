#include "blocks.h"

b8 block_passable(block_types type) {
    if (type == BLOCK_SUGAR_CANE || type == BLOCK_FLOWER_ROSE ||
        type == BLOCK_WATER) {
        return true;
    }
    return false;
}
