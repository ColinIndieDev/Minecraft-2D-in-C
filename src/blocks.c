#include "blocks.h"

block_data_t block_data[BLOCK_TYPES] = {
    {.uv = VEC2F(1, 0), .base_mining_dt = 0.6f,   .unbreakable = false, .passable = false}, // Grass Block
    {.uv = VEC2F(0, 0), .base_mining_dt = 0.6f,   .unbreakable = false, .passable = false}, // Dirt
    {.uv = VEC2F(2, 0), .base_mining_dt = 10.0f,  .unbreakable = false, .passable = false}, // Stone
    {.uv = VEC2F(0, 1), .base_mining_dt = 0.6f,   .unbreakable = false, .passable = false}, // Sand
    {.uv = VEC2F(1, 1), .base_mining_dt = -1.0f,  .unbreakable = true,  .passable = false}, // Bedrock
    {.uv = VEC2F(2, 1), .base_mining_dt = -1.0f,  .unbreakable = true,  .passable = true},  // Water
    {.uv = VEC2F(0, 2), .base_mining_dt = 0.1f,   .unbreakable = false, .passable = true},  // Rose
    {.uv = VEC2F(1, 2), .base_mining_dt = 0.3f,   .unbreakable = false, .passable = true},  // Sugar Cane
    {.uv = VEC2F(3, 0), .base_mining_dt = 2.0f,   .unbreakable = false, .passable = false}, // Oak Log
    {.uv = VEC2F(3, 1), .base_mining_dt = 0.3f,   .unbreakable = false, .passable = false}, // Oak Leaves
    {.uv = VEC2F(2, 2), .base_mining_dt = 0.6f,   .unbreakable = false, .passable = false}, // Gravel
    {.uv = VEC2F(3, 2), .base_mining_dt = 10.0f,  .unbreakable = false, .passable = false}, // Cobblestone
    {.uv = VEC2F(3, 3), .base_mining_dt = 2.0f,   .unbreakable = false, .passable = false}, // Oak Planks
    {.uv = VEC2F(2, 3), .base_mining_dt = 15.0f,  .unbreakable = false, .passable = false}, // Coal Ore
    {.uv = VEC2F(1, 3), .base_mining_dt = 15.0f,  .unbreakable = false, .passable = false}, // Iron Ore
    {.uv = VEC2F(0, 3), .base_mining_dt = 15.0f,  .unbreakable = false, .passable = false}, // Diamond Ore
    {.uv = VEC2F(0, 4), .base_mining_dt = 0.1f,   .unbreakable = false, .passable = true},  // Dandelion
    {.uv = VEC2F(1, 4), .base_mining_dt = 0.1f,   .unbreakable = false, .passable = true},  // Houstonia
    {.uv = VEC2F(2, 4), .base_mining_dt = 0.1f,   .unbreakable = false, .passable = true},  // Oxeye Daisy
    {.uv = VEC2F(3, 4), .base_mining_dt = 0.1f,   .unbreakable = false, .passable = true},  // Allium
    {.uv = VEC2F(4, 0), .base_mining_dt = 0.1f,   .unbreakable = false, .passable = true},  // Grass
};
