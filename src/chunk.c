#include "chunk.h"

#ifndef __EMSCRIPTEN__
#include <cpl/cpl.h>
#include <cpstd/cpbase.h>
#include <cpstd/cprng.h>
#else
#include "../cpstd/cpbase.h"
#include "../cpstd/cprng.h"
#include "../external/cpl.h"
#endif

u32 chunk_derive_seed(u32 base, u32 salt) {
    u32 h = base;
    h ^= salt + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
}

// {{{ Seeds

void chunk_gen_seed(map_noise_t *map_noise) {
    map_noise->terrain = fnlCreateState();
    map_noise->terrain.seed = cprng_rand_range(-I32_MAX + 1, I32_MAX);
    map_noise->terrain.noise_type = FNL_NOISE_OPENSIMPLEX2;
    map_noise->terrain.frequency = 0.01f;
    map_noise->terrain.fractal_type = FNL_FRACTAL_FBM;
    map_noise->terrain.octaves = 4;

    map_noise->tree_seed =
        (i32)chunk_derive_seed(CPM_ABS(map_noise->terrain.seed), 8471134);

    map_noise->tree_mask = fnlCreateState();
    map_noise->tree_mask.seed =
        (i32)chunk_derive_seed(CPM_ABS(map_noise->terrain.seed), 112744245);
    map_noise->tree_mask.noise_type = FNL_NOISE_OPENSIMPLEX2;
    map_noise->tree_mask.frequency = 0.003f;

    map_noise->caves = fnlCreateState();
    map_noise->caves.seed =
        (i32)chunk_derive_seed(CPM_ABS(map_noise->terrain.seed), 67676767);
    map_noise->caves.noise_type = FNL_NOISE_OPENSIMPLEX2;
    map_noise->caves.fractal_type = FNL_FRACTAL_RIDGED;
    map_noise->caves.frequency = 0.025f;
    map_noise->caves.octaves = 3;
    map_noise->caves.lacunarity = 2.0f;
    map_noise->caves.gain = 0.5f;
    map_noise->cave_mask = fnlCreateState();
    map_noise->cave_mask.seed =
        (i32)chunk_derive_seed(CPM_ABS(map_noise->terrain.seed), 123456789);
    map_noise->cave_mask.noise_type = FNL_NOISE_OPENSIMPLEX2;
    map_noise->cave_mask.fractal_type = FNL_FRACTAL_NONE;
    map_noise->cave_mask.frequency = 0.008f;

    map_noise->ores = fnlCreateState();
    map_noise->ores.seed =
        (i32)chunk_derive_seed(CPM_ABS(map_noise->terrain.seed), 9834881);
    map_noise->ores.noise_type = FNL_NOISE_CELLULAR;
    map_noise->ores.cellular_return_type = FNL_CELLULAR_RETURN_TYPE_CELLVALUE;
    map_noise->ores.frequency = 0.15f;
}

// }}}

// I do not understand this monstrosity too but it is like just a rand()
// function but depending on the seed
f32 chunk_gen_hash(i32 x, i32 y, i32 seed) {
    u32 h = CPM_ABS(seed) ^ (x * 73856093) ^ (y * 19349663);
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return (f32)(h & 0xFFFFFF) / 16777216.0f;
}

// {{{ Chunkgen Helper Functions

vec2f chunk_gen_terrain(chunk *c, map_noise_t *map_noise,
                        block_data_t *block_data, vec2f pos, u32 height, u32 x,
                        u32 y) {
    vec2f uv = VEC2F(0, 0);
    u32 bedrock_len =
        2 + (u32)(chunk_gen_hash((i32)((u32)(pos.x * CHUNK_SIZE) + x), 0,
                                 map_noise->terrain.seed - 1000) *
                  4);
    if (y == height - 1) {
        if (height <= SEA_LEVEL) {
            if (height <= SEA_LEVEL - 6) {
                uv = block_data[BLOCK_GRAVEL].uv;
            } else {
                uv = block_data[BLOCK_SAND].uv;
            }
            for (u32 w = y + 1; w < SEA_LEVEL; w++) {
                vec2f water_pos = VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                                        (MAX_CHUNK_HEIGHT - w) * BLOCK_SIZE);
                tilemap_add_tile(&c->tiles_passable, water_pos,
                                 VEC2F_INIT(BLOCK_SIZE),
                                 block_data[BLOCK_WATER].uv);
            }
        } else {
            uv = block_data[BLOCK_GRASS_BLOCK].uv;
        }
    } else if (y > height - 7) {
        if (height <= SEA_LEVEL) {
            if (height <= SEA_LEVEL - 6) {
                uv = block_data[BLOCK_GRAVEL].uv;
            } else {
                uv = block_data[BLOCK_SAND].uv;
            }
        } else {
            uv = block_data[BLOCK_DIRT].uv;
        }
    } else if (y < bedrock_len) {
        uv = block_data[BLOCK_BEDROCK].uv;
    } else {
        uv = block_data[BLOCK_STONE].uv;

        f32 ore_val = fnlGetNoise2D(
            &map_noise->ores, (f32)((u32)(pos.x * CHUNK_SIZE) + x), (f32)y);
        if (ore_val > 0.6f) {
            f32 spawn_chance =
                chunk_gen_hash((i32)((u32)(pos.x * CHUNK_SIZE) + x), (i32)y,
                               map_noise->ores.seed);

            if (y < DIAMOND_MAX_Y && spawn_chance < DIAMOND_SPAWN_CHANCE) {
                uv = block_data[BLOCK_DIAMOND_ORE].uv;
            } else if (y < IRON_MAX_Y && spawn_chance < IRON_SPAWN_CHANCE) {
                uv = block_data[BLOCK_IRON_ORE].uv;
            } else if (y < COAL_MAX_Y && spawn_chance < COAL_SPAWN_CHANCE) {
                uv = block_data[BLOCK_COAL_ORE].uv;
            }
        }
    }
    return uv;
}

void chunk_gen_foliage(chunk *c, map_noise_t *map_noise,
                       block_data_t *block_data, vec2f pos) {
    for (u32 x = 0; x < CHUNK_SIZE; x++) {
        f32 noise = (fnlGetNoise2D(&map_noise->terrain,
                                   (f32)((u32)(pos.x * CHUNK_SIZE) + x), 0) +
                     1.0f) *
                    0.5f;
        u32 height = MIN_TERRAIN_HEIGHT +
                     (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));

        if (cprng_rand() % 5 == 0) {
            if (height > SEA_LEVEL) {
                vec2f block_pos =
                    VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                          (MAX_CHUNK_HEIGHT - height) * BLOCK_SIZE);
                if (!tilemap_tile_exists(&c->tiles, block_pos)) {
                    tilemap_add_tile(&c->tiles_passable, block_pos,
                                     VEC2F_INIT(BLOCK_SIZE),
                                     block_data[BLOCK_FLOWER_ROSE].uv);
                }
            } else if (height == SEA_LEVEL) {
                for (u32 y = 0; y < cprng_rand_range(1, 5); y++) {
                    vec2f block_pos =
                        VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                              (MAX_CHUNK_HEIGHT - height) * BLOCK_SIZE);
                    if (!tilemap_tile_exists(&c->tiles, block_pos)) {
                        tilemap_add_tile(&c->tiles_passable, block_pos,
                                         VEC2F_INIT(BLOCK_SIZE),
                                         block_data[BLOCK_SUGAR_CANE].uv);
                    }
                }
            }
        }
    }
}

void chunk_gen_tree(chunk *c, map_noise_t *map_noise, block_data_t *block_data,
                    vec2f pos, u32 x) {
    u32 tree_height =
        2 + (i32)(chunk_gen_hash((i32)(pos.x + (f32)x), (i32)pos.y,
                                 (i32)map_noise->tree_seed + 5000) *
                  5.0f);
    for (u32 y = 0; y < tree_height; y++) {
        vec2f block_pos = VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                                (MAX_CHUNK_HEIGHT - pos.y - y) * BLOCK_SIZE);
        if (tilemap_tile_exists(&c->tiles, block_pos)) {
            tilemap_delete_tile(&c->tiles, block_pos);
        }
        if (tilemap_tile_exists(&c->tiles_passable, block_pos)) {
            tilemap_delete_tile(&c->tiles_passable, block_pos);
        }
        tilemap_add_tile(&c->tiles, block_pos, VEC2F_INIT(BLOCK_SIZE),
                         block_data[BLOCK_OAK_LOG].uv);
    }
    for (u32 ly = tree_height - 2; ly < tree_height - 2 + 4; ly++) {
        i32 leaf_radius = (ly == tree_height - 2 + 4 - 1) ? 1 : 2;

        for (i32 lx = -leaf_radius; lx <= leaf_radius; lx++) {
            i32 target_x = (i32)x + lx;
            if (target_x < 0 || target_x >= (i32)CHUNK_SIZE) {
                continue;
            }
            vec2f block_pos =
                VEC2F(((pos.x * CHUNK_SIZE) + target_x) * BLOCK_SIZE,
                      (MAX_CHUNK_HEIGHT - pos.y - ly) * BLOCK_SIZE);
            if (tilemap_tile_exists(&c->tiles_passable, block_pos)) {
                tilemap_delete_tile(&c->tiles_passable, block_pos);
            }
            tilemap_add_tile(&c->tiles, block_pos, VEC2F_INIT(BLOCK_SIZE),
                             block_data[BLOCK_OAK_LEAVES].uv);
        }
    }
}

void chunk_gen_trees(chunk *c, map_noise_t *map_noise, block_data_t *block_data,
                     vec2f pos) {
    for (u32 x = 0; x < CHUNK_SIZE; x++) {
        f32 noise = (fnlGetNoise2D(&map_noise->terrain,
                                   (f32)((u32)(pos.x * CHUNK_SIZE) + x), 0) +
                     1.0f) *
                    0.5f;
        u32 height = MIN_TERRAIN_HEIGHT +
                     (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));

        f32 mask_val = fnlGetNoise2D(&map_noise->tree_mask,
                                     (f32)((u32)(pos.x * CHUNK_SIZE) + x), 0);

        f32 spawn_chance = chunk_gen_hash((i32)(pos.x + (f32)x), (i32)height,
                                          (i32)map_noise->tree_seed);

        f32 probability = 0.0f;
        if (mask_val > 0.1f) {
            probability = 0.4f;
        } else {
            probability = 0.05f;
        }

        if (spawn_chance < probability &&
            chunk_gen_hash((i32)(pos.x + (f32)x - 1), (i32)height,
                           (i32)map_noise->tree_seed) > probability &&
            height > SEA_LEVEL) {
            chunk_gen_tree(c, map_noise, block_data, VEC2F(pos.x, height), x);
        }
    }
}

void chunk_gen_caves(chunk *c, map_noise_t *map_noise, vec2f pos, u32 height,
                     u32 x, u32 y) {
    if (y < MAX_CAVE_GEN_HEIGHT && y > MIN_CAVE_GEN_HEIGHT) {
        f32 cave_val = fnlGetNoise2D(
            &map_noise->caves, (f32)((u32)(pos.x * CHUNK_SIZE) + x), (f32)y);
        f32 mask_val =
            fnlGetNoise2D(&map_noise->cave_mask,
                          (f32)((u32)(pos.x * CHUNK_SIZE) + x), (f32)y);
        mask_val = (mask_val + 1.0f) * 0.5f;
        f32 depth = (f32)height - (f32)y;
        f32 surface_fade = CPM_CLAMP(depth / 10.0f, 0.0f, 1.0f);

        f32 dist_to_ceil = (f32)(MAX_CAVE_GEN_HEIGHT - y);
        f32 ceil_fade = CPM_CLAMP(depth / FADE_DISTANCE, 0.0f, 1.0f);

        f32 dist_to_bedrock = (f32)(y - MIN_CAVE_GEN_HEIGHT);
        f32 bedrock_fade =
            CPM_CLAMP(dist_to_bedrock / FADE_DISTANCE, 0.0f, 1.0f);

        f32 total_fade = surface_fade * ceil_fade * bedrock_fade;

        if (mask_val > 0.7f) {
            f32 dynamicThreshold = 0.9f - (mask_val * 0.15f);
            if (cave_val > dynamicThreshold * (1.0f - total_fade)) {
                tilemap_delete_tile(
                    &c->tiles, VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                                     (MAX_CHUNK_HEIGHT - y) * BLOCK_SIZE));
            }
        }
    }
}

// }}}

void chunk_gen(chunk *c, map_noise_t *map_noise, block_data_t *block_data,
               vec2f pos) {
    create_tilemap(&c->tiles, VEC2F(16, 16));
    tilemap_load_texture(&c->tiles, "assets/images/blocks/block_map.png",
                         FILTER_NEAREST);
    create_tilemap(&c->tiles_passable, VEC2F(16, 16));
    tilemap_load_texture(&c->tiles_passable,
                         "assets/images/blocks/block_map.png", FILTER_NEAREST);
    create_tilemap(&c->tiles_bg, VEC2F(16, 16));
    tilemap_load_texture(&c->tiles_bg, "assets/images/blocks/block_map.png",
                         FILTER_NEAREST);
    c->pos = pos;
    tilemap_begin_editing(&c->tiles);
    tilemap_begin_editing(&c->tiles_passable);
    tilemap_begin_editing(&c->tiles_bg);
    for (u32 x = 0; x < CHUNK_SIZE; x++) {
        f32 noise = (fnlGetNoise2D(&map_noise->terrain,
                                   (f32)((u32)(pos.x * CHUNK_SIZE) + x), 0) +
                     1.0f) *
                    0.5f;
        u32 height = MIN_TERRAIN_HEIGHT +
                     (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));
        u32 bedrock_len =
            2 + (u32)(chunk_gen_hash((i32)((u32)(pos.x * CHUNK_SIZE) + x), 0,
                                     map_noise->terrain.seed - 1000) *
                      4);
        for (u32 y = 0; y < height; y++) {
            vec2f uv =
                chunk_gen_terrain(c, map_noise, block_data, pos, height, x, y);

            vec2f tile_pos = VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                                   (MAX_CHUNK_HEIGHT - y) * BLOCK_SIZE);
            tilemap_add_tile(&c->tiles, tile_pos, VEC2F_INIT(BLOCK_SIZE), uv);
            if (!vec2f_cmp(uv, block_data[BLOCK_BEDROCK].uv)) {
                tilemap_add_tile(&c->tiles_bg, tile_pos, VEC2F_INIT(BLOCK_SIZE),
                                 uv);
            }

            chunk_gen_caves(c, map_noise, pos, height, x, y);
        }
    }
    chunk_gen_foliage(c, map_noise, block_data, pos);
    chunk_gen_trees(c, map_noise, block_data, pos);
}

void chunk_draw(chunk *c) {
    tilemap_draw(&c->tiles_bg, RGB(125, 125, 125));
    tilemap_draw(&c->tiles, WHITE);
}

void chunk_draw_passable(chunk *c) { tilemap_draw(&c->tiles_passable, WHITE); }
