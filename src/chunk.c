#include "chunk.h"
#include "blocktypes.h"
#include <cpstd/cprng.h>

#define CPL_IMPLEMENTATION
#define FNL_IMPL

void chunk_gen(chunk *c, vec2f pos, fnl_state *terrain) {
    create_tilemap(&c->tiles, VEC2F_INIT(16));
    tilemap_load_texture(&c->tiles, "assets/images/block_map.png",
                         FILTER_NEAREST);
    create_tilemap(&c->tiles_passable, VEC2F_INIT(16));
    tilemap_load_texture(&c->tiles_passable, "assets/images/block_map.png",
                         FILTER_NEAREST);
    c->pos = pos;
    tilemap_begin_editing(&c->tiles);
    tilemap_begin_editing(&c->tiles_passable);
    for (u32 x = 0; x < CHUNK_SIZE; x++) {
        f32 noise =
            (fnlGetNoise2D(terrain, (f32)((u32)(pos.x * CHUNK_SIZE) + x), 0) +
             1.0f) *
            0.5f;
        u32 height = MIN_TERRAIN_HEIGHT +
                     (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));
        u32 bedrock_height = cprng_rand_range(1, 6);
        for (u32 y = 0; y < height; y++) {
            vec2f uv;
            if (y == height - 1) {
                if (height <= SEA_LEVEL) {
                    uv = BLOCK_SAND;
                    for (u32 w = y + 1; w < SEA_LEVEL; w++) {
                        tilemap_add_tile(
                            &c->tiles_passable,
                            VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                                  (MAX_CHUNK_HEIGHT - w) * BLOCK_SIZE),
                            VEC2F_INIT(BLOCK_SIZE), BLOCK_WATER);
                    }
                } else {
                    uv = BLOCK_GRASS_BLOCK;
                }
            } else if (y > height - 7) {
                if (height <= SEA_LEVEL) {
                    uv = BLOCK_SAND;
                } else {
                    uv = BLOCK_DIRT;
                }
            } else if (y < bedrock_height) {
                uv = BLOCK_BEDROCK;
            } else {
                uv = BLOCK_STONE;
            }

            tilemap_add_tile(&c->tiles,
                             VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                                   (MAX_CHUNK_HEIGHT - y) * BLOCK_SIZE),
                             VEC2F_INIT(BLOCK_SIZE), uv);
        }
    }

    gen_trees(c, VEC2F(pos.x, pos.y), terrain);
    gen_foliage(c, VEC2F(pos.x, pos.y), terrain);
}

void gen_foliage(chunk *c, vec2f pos, fnl_state *terrain) {
    for (u32 x = 0; x < CHUNK_SIZE; x++) {
        f32 noise =
            (fnlGetNoise2D(terrain, (f32)((u32)(pos.x * CHUNK_SIZE) + x), 0) +
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
                        VEC2F_INIT(BLOCK_SIZE), BLOCK_FLOWER_ROSE);
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
                            VEC2F_INIT(BLOCK_SIZE), BLOCK_SUGAR_CANE);
                    }
                }
            }
        }
    }
}

void gen_tree(chunk *c, vec2f pos, u32 x) {
    u32 tree_height = cprng_rand_range(2, 7);
    for (u32 y = 0; y < tree_height; y++) {
        tilemap_add_tile(&c->tiles,
                         VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                               (MAX_CHUNK_HEIGHT - pos.y - y) * BLOCK_SIZE),
                         VEC2F_INIT(BLOCK_SIZE), BLOCK_OAK_LOG);
    }
    for (u32 ly = tree_height - 2; ly < tree_height - 2 + 4; ly++) {
        if (ly == tree_height - 2 + 4 - 1) {
            for (i32 lx = -1; lx < 2; lx++) {
                tilemap_add_tile(
                    &c->tiles,
                    VEC2F(((pos.x * CHUNK_SIZE) + x + lx) * BLOCK_SIZE,
                          (MAX_CHUNK_HEIGHT - pos.y - ly) * BLOCK_SIZE),
                    VEC2F_INIT(BLOCK_SIZE), BLOCK_OAK_LEAVES);
            }
        } else {
            for (i32 lx = -2; lx < 3; lx++) {
                tilemap_add_tile(
                    &c->tiles,
                    VEC2F(((pos.x * CHUNK_SIZE) + x + lx) * BLOCK_SIZE,
                          (MAX_CHUNK_HEIGHT - pos.y - ly) * BLOCK_SIZE),
                    VEC2F_INIT(BLOCK_SIZE), BLOCK_OAK_LEAVES);
            }
        }
    }
}

void gen_trees(chunk *c, vec2f pos, fnl_state *terrain) {
    for (u32 x = 0; x < CHUNK_SIZE; x++) {
        f32 noise =
            (fnlGetNoise2D(terrain, (f32)((u32)(pos.x * CHUNK_SIZE) + x), 0) +
             1.0f) *
            0.5f;
        u32 height = MIN_TERRAIN_HEIGHT +
                     (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));

        if (cprng_rand() % 5 == 0 && height > SEA_LEVEL) {
            gen_tree(c, VEC2F(pos.x, height), x);
        }
    }
}

void chunk_draw(chunk *c) { tilemap_draw(&c->tiles); }

void chunk_draw_passable(chunk *c) { tilemap_draw(&c->tiles_passable); }
