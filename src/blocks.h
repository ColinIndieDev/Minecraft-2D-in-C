#pragma once

#ifndef __EMSCRIPTEN__
#include <cpl/cpl.h>
#else
#include "../external/cpl.h"
#endif

typedef enum {
    BLOCK_GRASS_BLOCK,
    BLOCK_DIRT,
    BLOCK_STONE,
    BLOCK_SAND,
    BLOCK_BEDROCK,
    BLOCK_WATER,
    BLOCK_FLOWER_ROSE,
    BLOCK_SUGAR_CANE,
    BLOCK_OAK_LOG,
    BLOCK_OAK_LEAVES,
    BLOCK_GRAVEL,
    BLOCK_COBBLESTONE,
    BLOCK_OAK_PLANKS,
    BLOCK_COAL_ORE,
    BLOCK_IRON_ORE,
    BLOCK_DIAMOND_ORE,
    BLOCK_TYPES
} block_types;

typedef struct {
    vec2f uv;
    f32 base_mining_dt;
    b8 unbreakable;
    b8 passable;
} block_data_t;
