#pragma once

#include "blocks.h"
#include "noise.h"

#ifndef __EMSCRIPTEN__
#include <cpl/cpl.h>
#else
#include "../cpstd/cpmath.h"
#include "../external/cpl.h"
#endif

#define CHUNK_SIZE 16
#define MAX_CHUNK_HEIGHT 256

#define MAP_SIZE 1000

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

#define BLOCK_SIZE 75

typedef struct {
    vec2f pos;
    tilemap tiles;
    tilemap tiles_bg;
    tilemap tiles_passable;
} chunk;

typedef struct {
    fnl_state terrain;
    fnl_state caves;
    fnl_state cave_mask;
    fnl_state ores;
    fnl_state tree_mask;
    u32 tree_seed;
} map_noise_t;

void chunk_gen_seed(map_noise_t *map_noise);
void chunk_gen(chunk *c, map_noise_t *map_noise, block_data_t *block_data,
               vec2f pos);
void chunk_draw(chunk *c);
void chunk_draw_passable(chunk *c);
