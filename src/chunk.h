#pragma once

#include "blocks.h"
#include "noise.h"
#ifndef __EMSCRIPTEN__
#include <cpl/cpl.h>
#endif
#ifdef __EMSCRIPTEN__
#include "../cpstd/cpmath.h"
#include "../external/cpl.h"
#endif

#define CHUNK_SIZE 16
#define MAX_CHUNK_HEIGHT 256

#ifndef __EMSCRIPTEN__
#define MAP_SIZE 1000 /* 10000 */
#else
#define MAP_SIZE 1000
#endif

#define MIN_TERRAIN_HEIGHT 40
#define SEA_LEVEL 60
#define MAX_FIELD_HEIGHT 80
#define MAX_HILL_HEIGHT 100

#define BLOCK_SIZE 75

typedef struct {
    vec2f pos;
    tilemap tiles;
    tilemap tiles_passable;
} chunk;

typedef struct {
    fnl_state terrain;
} map_noise_t;

void gen_seed(map_noise_t *map_noise);
void gen_tree(chunk *c, vec2f pos, u32 x, block_data_t *block_data);
void gen_trees(chunk *c, vec2f pos, map_noise_t *map_noise,
               block_data_t *block_data);
void gen_foliage(chunk *c, vec2f pos, map_noise_t *map_noise,
                 block_data_t *block_data);
void chunk_gen(chunk *c, vec2f pos, map_noise_t *map_noise,
               block_data_t *block_data);
void chunk_draw(chunk *c);
void chunk_draw_passable(chunk *c);
