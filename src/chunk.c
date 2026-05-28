#include "chunk.h"
#ifndef __EMSCRIPTEN__
#include <cpl/cpl.h>
#include <cpstd/cpbase.h>
#include <cpstd/cprng.h>
#endif
#ifdef __EMSCRIPTEN__
#include "../cpstd/cpbase.h"
#include "../cpstd/cprng.h"
#include "../external/cpl.h"
#endif

void gen_seed(map_noise_t *map_noise) {
    map_noise->terrain = fnlCreateState();
    map_noise->terrain.seed = cprng_rand_range(-I32_MAX + 1, I32_MAX);
    map_noise->terrain.noise_type = FNL_NOISE_OPENSIMPLEX2;
    map_noise->terrain.frequency = 0.01f;
    map_noise->terrain.fractal_type = FNL_FRACTAL_FBM;
    map_noise->terrain.octaves = 4;
}

void chunk_gen(chunk *c, vec2f pos, map_noise_t *map_noise,
               block_data_t *block_data) {
    create_tilemap(&c->tiles, VEC2F_INIT(16));
    tilemap_load_texture(&c->tiles, "assets/images/blocks/block_map.png",
                         FILTER_NEAREST);
    create_tilemap(&c->tiles_passable, VEC2F_INIT(16));
    tilemap_load_texture(&c->tiles_passable,
                         "assets/images/blocks/block_map.png", FILTER_NEAREST);
    c->pos = pos;
    tilemap_begin_editing(&c->tiles);
    tilemap_begin_editing(&c->tiles_passable);
    for (u32 x = 0; x < CHUNK_SIZE; x++) {
        f32 noise = (fnlGetNoise2D(&map_noise->terrain,
                                   (f32)((u32)(pos.x * CHUNK_SIZE) + x), 0) +
                     1.0f) *
                    0.5f;
        u32 height = MIN_TERRAIN_HEIGHT +
                     (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));
        u32 bedrock_height = cprng_rand_range(1, 6);
        for (u32 y = 0; y < height; y++) {
            vec2f uv;
            if (y == height - 1) {
                if (height <= SEA_LEVEL) {
                    uv = block_data[BLOCK_SAND].uv;
                    for (u32 w = y + 1; w < SEA_LEVEL; w++) {
                        tilemap_add_tile(
                            &c->tiles_passable,
                            VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                                  (MAX_CHUNK_HEIGHT - w) * BLOCK_SIZE),
                            VEC2F_INIT(BLOCK_SIZE), block_data[BLOCK_WATER].uv);
                    }
                } else {
                    uv = block_data[BLOCK_GRASS_BLOCK].uv;
                }
            } else if (y > height - 7) {
                if (height <= SEA_LEVEL) {
                    uv = block_data[BLOCK_SAND].uv;
                } else {
                    uv = block_data[BLOCK_DIRT].uv;
                }
            } else if (y < bedrock_height) {
                uv = block_data[BLOCK_BEDROCK].uv;
            } else {
                uv = block_data[BLOCK_STONE].uv;
            }

            tilemap_add_tile(&c->tiles,
                             VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                                   (MAX_CHUNK_HEIGHT - y) * BLOCK_SIZE),
                             VEC2F_INIT(BLOCK_SIZE), uv);
        }
    }
    gen_foliage(c, VEC2F(pos.x, pos.y), map_noise, block_data);
    gen_trees(c, VEC2F(pos.x, pos.y), map_noise, block_data);
}

void gen_foliage(chunk *c, vec2f pos, map_noise_t *map_noise,
                 block_data_t *block_data) {
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
                    tilemap_add_tile(
                        &c->tiles_passable,
                        VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                              (MAX_CHUNK_HEIGHT - height) * BLOCK_SIZE),
                        VEC2F_INIT(BLOCK_SIZE),
                        block_data[BLOCK_FLOWER_ROSE].uv);
                }
            } else if (height == SEA_LEVEL) {
                for (u32 y = 0; y < cprng_rand_range(1, 5); y++) {
                    vec2f block_pos =
                        VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                              (MAX_CHUNK_HEIGHT - height) * BLOCK_SIZE);
                    if (!tilemap_tile_exists(&c->tiles, block_pos)) {
                        tilemap_add_tile(
                            &c->tiles_passable,
                            VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                                  (MAX_CHUNK_HEIGHT - height - y) * BLOCK_SIZE),
                            VEC2F_INIT(BLOCK_SIZE),
                            block_data[BLOCK_SUGAR_CANE].uv);
                    }
                }
            }
        }
    }
}

void gen_tree(chunk *c, vec2f pos, u32 x, block_data_t *block_data) {
    u32 tree_height = cprng_rand_range(2, 7);
    for (u32 y = 0; y < tree_height; y++) {
        if (tilemap_tile_exists(
                &c->tiles,
                VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                      (MAX_CHUNK_HEIGHT - pos.y - y) * BLOCK_SIZE))) {
            tilemap_delete_tile(
                &c->tiles, VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                                 (MAX_CHUNK_HEIGHT - pos.y - y) * BLOCK_SIZE));
        }
        if (tilemap_tile_exists(
                &c->tiles_passable,
                VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                      (MAX_CHUNK_HEIGHT - pos.y - y) * BLOCK_SIZE))) {
            tilemap_delete_tile(
                &c->tiles_passable,
                VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                      (MAX_CHUNK_HEIGHT - pos.y - y) * BLOCK_SIZE));
        }
        tilemap_add_tile(&c->tiles,
                         VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                               (MAX_CHUNK_HEIGHT - pos.y - y) * BLOCK_SIZE),
                         VEC2F_INIT(BLOCK_SIZE), block_data[BLOCK_OAK_LOG].uv);
    }
    for (u32 ly = tree_height - 2; ly < tree_height - 2 + 4; ly++) {
        i32 leaf_radius = (ly == tree_height - 2 + 4 - 1) ? 1 : 2;

        for (i32 lx = -leaf_radius; lx <= leaf_radius; lx++) {
            i32 target_x = (i32)x + lx;

            if (target_x < 0 || target_x >= (i32)CHUNK_SIZE) {
                continue;
            }
            if (tilemap_tile_exists(
                    &c->tiles_passable,
                    VEC2F(((pos.x * CHUNK_SIZE) + target_x) * BLOCK_SIZE,
                          (MAX_CHUNK_HEIGHT - pos.y - ly) * BLOCK_SIZE))) {
                tilemap_delete_tile(
                    &c->tiles_passable,
                    VEC2F(((pos.x * CHUNK_SIZE) + target_x) * BLOCK_SIZE,
                          (MAX_CHUNK_HEIGHT - pos.y - ly) * BLOCK_SIZE));
            }
            tilemap_add_tile(
                &c->tiles,
                VEC2F(((pos.x * CHUNK_SIZE) + target_x) * BLOCK_SIZE,
                      (MAX_CHUNK_HEIGHT - pos.y - ly) * BLOCK_SIZE),
                VEC2F_INIT(BLOCK_SIZE), block_data[BLOCK_OAK_LEAVES].uv);
        }
    }
}

void gen_trees(chunk *c, vec2f pos, map_noise_t *map_noise,
               block_data_t *block_data) {
    for (u32 x = 0; x < CHUNK_SIZE; x++) {
        f32 noise = (fnlGetNoise2D(&map_noise->terrain,
                                   (f32)((u32)(pos.x * CHUNK_SIZE) + x), 0) +
                     1.0f) *
                    0.5f;
        u32 height = MIN_TERRAIN_HEIGHT +
                     (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));

        if (cprng_rand() % 5 == 0 && height > SEA_LEVEL) {
            gen_tree(c, VEC2F(pos.x, height), x, block_data);
        }
    }
}

void chunk_draw(chunk *c) { tilemap_draw(&c->tiles); }

void chunk_draw_passable(chunk *c) { tilemap_draw(&c->tiles_passable); }
