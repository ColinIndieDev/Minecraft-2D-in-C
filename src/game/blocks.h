#pragma once

#include <cpl/cpl.h>

typedef enum : uint8_t {
    BLOCK_GRASS_BLOCK = 0,
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
    BLOCK_FLOWER_DANDELION,
    BLOCK_FLOWER_HOUSTONIA,
    BLOCK_FLOWER_OXEYE_DAISY,
    BLOCK_FLOWER_ALLIUM,
    BLOCK_GRASS,
    BLOCK_TYPE_T_SIZE
} block_type_t;

typedef struct {
    vec2f uv;
    float base_mining_dt;
    bool unbreakable;
    bool passable;
} block_data_t;

block_data_t *blocks_get_block_data(block_type_t type);
