#include "chunk.h"
#include "blocks.h"
#include "player.h"

#include <cpl/cpl.h>
#include <cpstd/mathplus.h>
#include <cpstd/rand.h>
#include <pthread.h>

pthread_t chunk_gen_workers[CHUNK_GEN_THREADS];
pthread_mutex_t chunk_gen_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t chunk_gen_cond = PTHREAD_COND_INITIALIZER;
bool threads_running = true;

uint32_t left_most_chunk = 0;
uint32_t right_most_chunk = 0;
int start_chunk = 0;
int end_chunk = 0;

chunk_entry *chunks = NULL;
chunk_t **chunk_gen_queue = NULL;
map_noise_t map_noise;
worker_data_t chunk_gen_worker_data = {
    .map_noise = &map_noise,
    .block_data = NULL
};

chunk_entry *chunk_get_chunkmap() {
    return chunks;
}

uint32_t chunk_get_leftmost_idx() {
    return left_most_chunk;
}

uint32_t chunk_get_rightmost_idx() {
    return right_most_chunk;
}

map_noise_t *chunk_get_map_noise() {
    return &map_noise;
}

#pragma region Seed

uint32_t chunk_derive_seed(uint32_t base, uint32_t salt) {
    uint32_t h = base;
    h ^= salt + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
}

void chunk_gen_seed() {
    map_noise.terrain = fnlCreateState();
    map_noise.terrain.seed = pcg_rand_range(-INT32_MAX + 1, INT32_MAX);
    map_noise.terrain.noise_type = FNL_NOISE_OPENSIMPLEX2;
    map_noise.terrain.frequency = 0.01f;
    map_noise.terrain.fractal_type = FNL_FRACTAL_FBM;
    map_noise.terrain.octaves = 4;

    map_noise.tree_seed = chunk_derive_seed(abs(map_noise.terrain.seed), 8471134);

    map_noise.tree_mask = fnlCreateState();
    map_noise.tree_mask.seed = (int)chunk_derive_seed(abs(map_noise.terrain.seed), 112744245);
    map_noise.tree_mask.noise_type = FNL_NOISE_OPENSIMPLEX2;
    map_noise.tree_mask.frequency = 0.003f;

    map_noise.caves = fnlCreateState();
    map_noise.caves.seed = (int)chunk_derive_seed(abs(map_noise.terrain.seed), 67676767);
    map_noise.caves.noise_type = FNL_NOISE_OPENSIMPLEX2;
    map_noise.caves.fractal_type = FNL_FRACTAL_RIDGED;
    map_noise.caves.frequency = 0.025f;
    map_noise.caves.octaves = 3;
    map_noise.caves.lacunarity = 2.0f;
    map_noise.caves.gain = 0.5f;
    map_noise.cave_mask = fnlCreateState();
    map_noise.cave_mask.seed = (int)chunk_derive_seed(abs(map_noise.terrain.seed), 123456789);
    map_noise.cave_mask.noise_type = FNL_NOISE_OPENSIMPLEX2;
    map_noise.cave_mask.fractal_type = FNL_FRACTAL_NONE;
    map_noise.cave_mask.frequency = 0.008f;

    map_noise.ores = fnlCreateState();
    map_noise.ores.seed = (int)chunk_derive_seed(abs(map_noise.terrain.seed), 9834881);
    map_noise.ores.noise_type = FNL_NOISE_CELLULAR;
    map_noise.ores.cellular_return_type = FNL_CELLULAR_RETURN_TYPE_CELLVALUE;
    map_noise.ores.frequency = 0.15f;
}

#pragma endregion

#pragma region chunk_gen() Helper

// I do not understand this monstrosity too but it is like just a rand()
// function but depending on the seed
float chunk_gen_hash(int x, int y, int seed) {
    uint32_t h = abs(seed) ^ (x * 73856093) ^ (y * 19349663);
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return (float)(h & 0xFFFFFF) / 16777216.0f;
}

vec2f chunk_gen_terrain(chunk_t *c, vec2f pos, uint32_t height, uint32_t x, uint32_t y) {
    vec2f uv = VEC2F(0, 0);
    uint32_t bedrock_len = 2 + (uint32_t)(chunk_gen_hash((int)((uint32_t)(pos.x * CHUNK_SIZE) + x), 0, map_noise.terrain.seed - 1000) * 4);
    if (y == height - 1) {
        if (height <= SEA_LEVEL) {
            if (height <= SEA_LEVEL - 6) {
                uv = blocks_get_block_data(BLOCK_GRAVEL)->uv;
            } else {
                uv = blocks_get_block_data(BLOCK_SAND)->uv;
            }
            for (uint32_t w = y + 1; w < SEA_LEVEL; w++) {
                vec2f water_pos = vec2f_float_mul(VEC2F(((pos.x * CHUNK_SIZE) + x), (MAX_CHUNK_HEIGHT - w)), BLOCK_SIZE);
                tilemap_add_tile(&c->tiles_passable, water_pos, VEC2F(BLOCK_SIZE, BLOCK_SIZE), blocks_get_block_data(BLOCK_WATER)->uv);
            }
        } else {
            uv = blocks_get_block_data(BLOCK_GRASS_BLOCK)->uv;
        }
    } else if (y > height - 7) {
        if (height <= SEA_LEVEL) {
            if (height <= SEA_LEVEL - 6) {
                uv = blocks_get_block_data(BLOCK_GRAVEL)->uv;
            } else {
                uv = blocks_get_block_data(BLOCK_SAND)->uv;
            }
        } else {
            uv = blocks_get_block_data(BLOCK_DIRT)->uv;
        }
    } else if (y < bedrock_len) {
        uv = blocks_get_block_data(BLOCK_BEDROCK)->uv;
    } else {
        uv = blocks_get_block_data(BLOCK_STONE)->uv;

        float ore_val = fnlGetNoise2D(&map_noise.ores, (float)((uint32_t)(pos.x * CHUNK_SIZE) + x), (float)y);
        if (ore_val > 0.6f) {
            float spawn_chance = chunk_gen_hash((int)((uint32_t)(pos.x * CHUNK_SIZE) + x), (int)y, map_noise.ores.seed);

            if (y < DIAMOND_MAX_Y && spawn_chance < DIAMOND_SPAWN_CHANCE) {
                uv = blocks_get_block_data(BLOCK_DIAMOND_ORE)->uv;
            } else if (y < IRON_MAX_Y && spawn_chance < IRON_SPAWN_CHANCE) {
                uv = blocks_get_block_data(BLOCK_IRON_ORE)->uv;
            } else if (y < COAL_MAX_Y && spawn_chance < COAL_SPAWN_CHANCE) {
                uv = blocks_get_block_data(BLOCK_COAL_ORE)->uv;
            }
        }
    }
    return uv;
}

void chunk_gen_foliage(chunk_t *c, vec2f pos) {
    for (uint32_t x = 0; x < CHUNK_SIZE; x++) {
        float noise = (fnlGetNoise2D(&map_noise.terrain, (float)((uint32_t)(pos.x * CHUNK_SIZE) + x), 0) + 1.0f) * 0.5f;
        uint32_t height = MIN_TERRAIN_HEIGHT + (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));

        float tree_mask_val = fnlGetNoise2D(&map_noise.tree_mask, (float)((uint32_t)(pos.x * CHUNK_SIZE) + x), 0);

        float probability = 0.0f;
        if (tree_mask_val > 0.1f) {
            probability = 0.2f;
        } else {
            probability = 0.7f;
        }

        float spawn_chance = chunk_gen_hash((int)(pos.x + (float)x), (int)height, (int)map_noise.tree_seed);

        if (spawn_chance < probability) {
            if (height > SEA_LEVEL) {
                vec2f block_pos = vec2f_float_mul(VEC2F(((pos.x * CHUNK_SIZE) + x), (MAX_CHUNK_HEIGHT - height)), BLOCK_SIZE);

                if (!tilemap_tile_exists(&c->tiles, block_pos)) {
                    uint32_t foliage_id = BLOCK_FLOWER_ROSE;
                    float grass_probability = chunk_gen_hash((int)(pos.x + (float)x), (int)height, map_noise.terrain.seed + 135);
                    if (grass_probability > 0.2f) {
                        foliage_id = BLOCK_GRASS;
                    } else {
                        uint32_t flower_variant = (uint32_t)(ceilf(4.0f * 
                                                  chunk_gen_hash((int)(pos.x + (float)x), (int)height, map_noise.terrain.seed - 10914)));
                        switch (flower_variant) {
                        case 1:
                            foliage_id = BLOCK_FLOWER_DANDELION;
                            break;
                        case 2:
                            foliage_id = BLOCK_FLOWER_HOUSTONIA;
                            break;
                        case 3:
                            foliage_id = BLOCK_FLOWER_OXEYE_DAISY;
                            break;
                        case 4:
                            foliage_id = BLOCK_FLOWER_ALLIUM;
                            break;
                        default:
                            break;
                        }
                    }
                    tilemap_add_tile(&c->tiles_passable, block_pos, VEC2F(BLOCK_SIZE, BLOCK_SIZE), blocks_get_block_data(foliage_id)->uv);
                }
            } else if (height == SEA_LEVEL) {
                float level = 1.0f + (3.0f * chunk_gen_hash((int)(pos.x + (float)x), (int)height, (int)map_noise.tree_seed - 1383));
                for (uint32_t y = 0; y < (uint32_t)level; y++) {
                    vec2f block_pos = vec2f_float_mul(VEC2F(((pos.x * CHUNK_SIZE) + x), (MAX_CHUNK_HEIGHT - height - y)), BLOCK_SIZE);

                    if (!tilemap_tile_exists(&c->tiles, block_pos)) {
                        tilemap_add_tile(&c->tiles_passable, block_pos, VEC2F(BLOCK_SIZE, BLOCK_SIZE), blocks_get_block_data(BLOCK_SUGAR_CANE)->uv);
                    }
                }
            }
        }
    }
}

#pragma region chunk_gen_trees() Helper

void chunk_gen_tree(chunk_t *c, vec2f pos, uint32_t x) {
    uint32_t tree_height = 2 + (int)(chunk_gen_hash((int)(pos.x + (float)x), (int)pos.y, (int)map_noise.tree_seed + 5000) * 5.0f);
    for (uint32_t y = 0; y < tree_height; y++) {
        vec2f block_pos = vec2f_float_mul(VEC2F(((pos.x * CHUNK_SIZE) + x), (MAX_CHUNK_HEIGHT - pos.y - y)), BLOCK_SIZE);

        if (tilemap_tile_exists(&c->tiles, block_pos)) {
            tilemap_delete_tile(&c->tiles, block_pos);
        }
        if (tilemap_tile_exists(&c->tiles_passable, block_pos)) {
            tilemap_delete_tile(&c->tiles_passable, block_pos);
        }
        tilemap_add_tile(&c->tiles, block_pos, VEC2F(BLOCK_SIZE, BLOCK_SIZE), blocks_get_block_data(BLOCK_OAK_LOG)->uv);
    }
    for (uint32_t ly = tree_height - 2; ly < tree_height - 2 + 4; ly++) {
        int leaf_radius = (ly == tree_height - 2 + 4 - 1) ? 1 : 2;
        for (int lx = -leaf_radius; lx <= leaf_radius; lx++) {
            int target_x = (int)x + lx;
            if (target_x < 0 || target_x >= (int)CHUNK_SIZE) {
                continue;
            }
            vec2f block_pos = vec2f_float_mul(VEC2F(((pos.x * CHUNK_SIZE) + target_x), (MAX_CHUNK_HEIGHT - pos.y - ly)), BLOCK_SIZE);

            if (tilemap_tile_exists(&c->tiles_passable, block_pos)) {
                tilemap_delete_tile(&c->tiles_passable, block_pos);
            }
            tilemap_add_tile(&c->tiles, block_pos, VEC2F(BLOCK_SIZE, BLOCK_SIZE), blocks_get_block_data(BLOCK_OAK_LEAVES)->uv);
        }
    }
}

bool chunk_gen_can_gen_tree(float spawn_chance, float probability, float pos_x, uint32_t x, uint32_t height) {
    return spawn_chance < probability && chunk_gen_hash((int)(pos_x + (float)x - 1), (int)height, (int)map_noise.tree_seed) > probability 
           && height > SEA_LEVEL;
}

#pragma endregion

void chunk_gen_trees(chunk_t *c, vec2f pos) {
    for (uint32_t x = 0; x < CHUNK_SIZE; x++) {
        float noise = (fnlGetNoise2D(&map_noise.terrain, (float)((uint32_t)(pos.x * CHUNK_SIZE) + x), 0) + 1.0f) * 0.5f;
        uint32_t height = MIN_TERRAIN_HEIGHT + (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));
        float mask_val = fnlGetNoise2D(&map_noise.tree_mask, (float)((uint32_t)(pos.x * CHUNK_SIZE) + x), 0);
        float spawn_chance = chunk_gen_hash((int)(pos.x + (float)x), (int)height, (int)map_noise.tree_seed);

        float probability = 0.0f;
        if (mask_val > 0.1f) {
            probability = 0.4f;
        } else {
            probability = 0.05f;
        }

        if (chunk_gen_can_gen_tree(spawn_chance, probability, pos.x, x, height)) {
            chunk_gen_tree(c, VEC2F(pos.x, height), x);
        }
    }
}

void chunk_gen_caves(chunk_t *c, vec2f pos, uint32_t height, uint32_t x, uint32_t y) {
    if (y < MAX_CAVE_GEN_HEIGHT && y > MIN_CAVE_GEN_HEIGHT) {
        float cave_val = fnlGetNoise2D(&map_noise.caves, (float)((uint32_t)(pos.x * CHUNK_SIZE) + x), (float)y);
        float mask_val = fnlGetNoise2D(&map_noise.cave_mask, (float)((uint32_t)(pos.x * CHUNK_SIZE) + x), (float)y);
        mask_val = (mask_val + 1.0f) * 0.5f;
        float depth = (float)height - (float)y;
        float surface_fade = math_clamp(depth / 10.0f, 0.0f, 1.0f);

        float dist_to_ceil = (float)(MAX_CAVE_GEN_HEIGHT - y);
        float ceil_fade = math_clamp(depth / FADE_DISTANCE, 0.0f, 1.0f);

        float dist_to_bedrock = (float)(y - MIN_CAVE_GEN_HEIGHT);
        float bedrock_fade = math_clamp(dist_to_bedrock / FADE_DISTANCE, 0.0f, 1.0f);

        float total_fade = surface_fade * ceil_fade * bedrock_fade;

        if (mask_val > 0.7f) {
            float dynamic_threshold = 0.9f - (mask_val * 0.15f);
            if (cave_val > dynamic_threshold * (1.0f - total_fade)) {
                vec2f block_pos = vec2f_float_mul(VEC2F(((pos.x * CHUNK_SIZE) + x), (MAX_CHUNK_HEIGHT - y)), BLOCK_SIZE);
                tilemap_delete_tile(&c->tiles, block_pos);
            }
        }
    }
}

#pragma endregion

#pragma region Threads

void *chunk_gen_loop(void *arg) {
    worker_data_t *data = (worker_data_t *)arg;
    while (true) {
        pthread_mutex_lock(&chunk_gen_queue_mutex);
        while (queue_empty(*(data->queue)) && threads_running) {
            pthread_cond_wait(&chunk_gen_cond, &chunk_gen_queue_mutex);
        }

        if (!threads_running && queue_empty(*(data->queue))) {
            pthread_mutex_unlock(&chunk_gen_queue_mutex);
            break;
        }

        chunk_t *c = queue_pop(*(data->queue));
        pthread_mutex_unlock(&chunk_gen_queue_mutex);
        chunk_gen(c);
        tilemap_check_collidable_tiles(&c->tiles,
                VEC2F(BLOCK_SIZE, BLOCK_SIZE));
        c->ready = true;
    }
    return NULL;
}

void chunk_init_threads() {
    chunk_gen_queue = queue_init(chunk_gen_queue, 10);
    chunk_gen_worker_data.queue = &chunk_gen_queue;
    chunk_gen_worker_data.block_data = blocks_get_block_data(0);

    for (uint32_t i = 0; i < CHUNK_GEN_THREADS; i++) {
        if (pthread_create(&chunk_gen_workers[i], NULL, chunk_gen_loop, (void *)&chunk_gen_worker_data)) {
            fprintf(stderr, "Failed to create worker thread!\n");
            exit(-1);
        }
    }
}

void chunk_close_threads() {
    pthread_mutex_lock(&chunk_gen_queue_mutex);
    threads_running = false;
    pthread_mutex_unlock(&chunk_gen_queue_mutex);

    pthread_cond_broadcast(&chunk_gen_cond);
    for (uint32_t i = 0; i < CHUNK_GEN_THREADS; i++) {
        pthread_join(chunk_gen_workers[i], NULL);
    }

    pthread_mutex_destroy(&chunk_gen_queue_mutex);
    pthread_cond_destroy(&chunk_gen_cond);
    queue_destroy(*(chunk_gen_worker_data.queue));
}

#pragma endregion

void chunk_gen_init() {
    chunks = hm_init(chunks, 10);

    chunk_gen_seed();

    uint32_t spawn_chunk_x = player_set_spawn_point(&map_noise.terrain);
    left_most_chunk = spawn_chunk_x - 1;
    right_most_chunk = spawn_chunk_x + 1;
    for (uint32_t i = left_most_chunk; i <= right_most_chunk; i++) {
        chunk_t *new_chunk = malloc(sizeof(chunk_t));
        new_chunk->pos = VEC2F(i, 0);
        new_chunk->ready = false;
        hm_put(chunks, i, new_chunk);
        chunk_gen_gl(new_chunk);
        queue_push(chunk_gen_queue, new_chunk);
        pthread_cond_signal(&chunk_gen_cond);
    }
}

#pragma region Generation

void chunk_gen_gl(chunk_t *c) {
    assert(c);

    tilemap_create(&c->tiles, VEC2F(16, 16));
    tilemap_load_texture(&c->tiles, "assets/images/blocks/block_map.png", FILTER_NEAREST);
    tilemap_create(&c->tiles_passable, VEC2F(16, 16));
    tilemap_load_texture(&c->tiles_passable, "assets/images/blocks/block_map.png", FILTER_NEAREST);
    tilemap_create(&c->tiles_bg, VEC2F(16, 16));
    tilemap_load_texture(&c->tiles_bg, "assets/images/blocks/block_map.png", FILTER_NEAREST);
}

void chunk_gen(chunk_t *c) {
    assert(c);
    assert(map_noise);
    assert(block_data);

    tilemap_begin_editing(&c->tiles);
    tilemap_begin_editing(&c->tiles_passable);
    tilemap_begin_editing(&c->tiles_bg);
    for (uint32_t x = 0; x < CHUNK_SIZE; x++) {
        float noise = (fnlGetNoise2D(&map_noise.terrain, (float)((uint32_t)(c->pos.x * CHUNK_SIZE) + x), 0) + 1.0f) * 0.5f;
        uint32_t height = MIN_TERRAIN_HEIGHT + (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));
        for (uint32_t y = 0; y < height; y++) {
            vec2f uv = chunk_gen_terrain(c, c->pos, height, x, y);
            vec2f tile_pos = vec2f_float_mul(VEC2F(((c->pos.x * CHUNK_SIZE) + x), (MAX_CHUNK_HEIGHT - y)), BLOCK_SIZE);

            tilemap_add_tile(&c->tiles, tile_pos, VEC2F(BLOCK_SIZE, BLOCK_SIZE), uv);
            if (!vec2f_cmp(uv, blocks_get_block_data(BLOCK_BEDROCK)->uv)) {
                tilemap_add_tile(&c->tiles_bg, tile_pos, VEC2F(BLOCK_SIZE, BLOCK_SIZE), uv);
            }

            chunk_gen_caves(c, c->pos, height, x, y);
        }
    }
    chunk_gen_foliage(c, c->pos);
    chunk_gen_trees(c, c->pos);
}

void chunk_calc_chunks_to_render() {
    float inv_block_scale = 1.0f / (CHUNK_SIZE * BLOCK_SIZE);

    start_chunk = (int)(get_cam_2D()->pos.x * (1.0f / (CHUNK_SIZE * BLOCK_SIZE)));
    end_chunk = (int)((get_cam_2D()->pos.x + ((float)get_screen_width() * (1.0f / get_cam_2D()->zoom))) * (1.0f / (CHUNK_SIZE * BLOCK_SIZE)));

    if (start_chunk < 0) {
        start_chunk = 0;
    }
    if (end_chunk >= MAP_SIZE) {
        end_chunk = MAP_SIZE - 1;
    }
}

void chunk_gen_chunks_req() {
    for (int i = start_chunk; i <= end_chunk; i++) {
        if (i >= 0 && i < MAP_SIZE) {
            chunk_t *existing_chunk = NULL;
            if (hm_get(chunks, i)) {
                existing_chunk = *hm_get(chunks, i);
            }

            if (!existing_chunk) {
                chunk_t *new_chunk = malloc(sizeof(chunk_t));
                new_chunk->pos = VEC2F(i, 0);
                new_chunk->ready = false;
                hm_put(chunks, i, new_chunk);
                if (new_chunk != NULL) {
                    chunk_gen_gl(new_chunk);
                    queue_push(chunk_gen_queue, new_chunk);
                    pthread_cond_signal(&chunk_gen_cond);
                }
                if (i < left_most_chunk) {
                    left_most_chunk = i;
                }
                if (i > right_most_chunk) {
                    right_most_chunk = i;
                }
            }            
        }
    }
}

#pragma endregion

#pragma region Drawing

void chunk_draw_chunk(chunk_t *c) {
    assert(c);

    tilemap_draw(&c->tiles_bg, RGB(125, 125, 125));
    tilemap_draw(&c->tiles, WHITE);
}

void chunk_draw_chunk_passable(chunk_t *c) { 
    assert(c);

    tilemap_draw(&c->tiles_passable, WHITE); 
}

void chunk_draw_chunks() {
    for (int i = start_chunk; i <= end_chunk; i++) {
        if (i >= left_most_chunk && i <= right_most_chunk) {
            chunk_t **c_ptr = hm_get(chunks, i);
            chunk_t *c = c_ptr ? *c_ptr : NULL;
            if (c && c->ready) {
                chunk_draw_chunk(c);
            }
        }
    }
}

void chunk_draw_chunks_passable() {
    for (int i = start_chunk; i <= end_chunk; i++) {
        if (i >= left_most_chunk && i <= right_most_chunk) {
            chunk_t **c_ptr = hm_get(chunks, i);
            chunk_t *c = c_ptr ? *c_ptr : NULL;
            if (c && c->ready) {
                chunk_draw_chunk_passable(c);
            }
        }
    }
}

#pragma endregion
