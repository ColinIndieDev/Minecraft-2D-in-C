#pragma once

#include "blocks.h"
#include "noise.h"

#include <cpl/cpl.h>
#include <cpstd/queue.h>

#define MAP_SIZE 10000
#define BLOCK_SIZE 75
#define CHUNK_SIZE 16
#define MAX_CHUNK_HEIGHT 256

#define MIN_TERRAIN_HEIGHT 40
#define SEA_LEVEL 60
#define MAX_FIELD_HEIGHT 80
#define MAX_HILL_HEIGHT 100

#define COAL_SPAWN_CHANCE 0.15f
#define COAL_MAX_Y 60
#define IRON_SPAWN_CHANCE 0.08f
#define IRON_MAX_Y 40
#define DIAMOND_SPAWN_CHANCE 0.02f
#define DIAMOND_MAX_Y 12

#define MAX_CAVE_GEN_HEIGHT MIN_TERRAIN_HEIGHT
#define MIN_CAVE_GEN_HEIGHT 10
#define FADE_DISTANCE 8.0f

#define CHUNK_GEN_THREADS 4

#define EXTERN_CHUNK_H_VARIABLES extern pthread_cond_t chunk_gen_cond;

typedef struct {
    vec2f pos;
    tilemap tiles;
    tilemap tiles_bg;
    tilemap tiles_passable;
    bool ready;
} chunk_t;

typedef struct {
    fnl_state terrain;
    fnl_state caves;
    fnl_state cave_mask;
    fnl_state ores;
    fnl_state tree_mask;
    uint32_t tree_seed;
} map_noise_t;

typedef struct {
    map_noise_t *map_noise;
    block_data_t *block_data;
    chunk_t ***queue;
} worker_data_t;

void chunk_gen_seed(map_noise_t *map_noise);
void chunk_gen_gl(chunk_t *c);
void chunk_gen(chunk_t *c, map_noise_t *map_noise, block_data_t *block_data);
void chunk_draw(chunk_t *c);
void chunk_draw_passable(chunk_t *c);

void chunk_init_threads(worker_data_t *data);
void chunk_close_threads(worker_data_t *data);
void *chunk_gen_loop(void *arg);
