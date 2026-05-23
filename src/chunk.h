#pragma once

#include <cpl/cpl.h>
#include "noise.h"

#define CHUNK_SIZE 16
#define MAX_CHUNK_HEIGHT 256

#define MAP_SIZE 1000 /* 10000 */

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
void gen_tree(chunk *c, vec2f pos, u32 x);
void gen_trees(chunk *c, vec2f pos, fnl_state *terrain);
void gen_foliage(chunk *c, vec2f pos, fnl_state *terrain);
void chunk_gen(chunk *c, vec2f pos, fnl_state *terrain);
void chunk_draw(chunk *c);
void chunk_draw_passable(chunk *c);
