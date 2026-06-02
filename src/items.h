#pragma once

#include "chunk.h"

#ifndef __EMSCRIPTEN__
#include <cpl/cpl.h>
#else
#include "../external/cpl.h"
#endif

#define EXTERN_ITEMS_H_VARIABLES extern item_data_t item_data[ITEM_TYPES];

#define ITEM_DROP_LIFETIME (5 * 60)
#define MAX_ANIM_OFFSET 10
#define ITEM_TEX_PATH "assets/images/items/"

typedef enum {
    ITEM_NONE,
    ITEM_GRASS_BLOCK,
    ITEM_DIRT,
    ITEM_STONE,
    ITEM_SAND,
    ITEM_BEDROCK,
    ITEM_FLOWER_ROSE,
    ITEM_SUGAR_CANE,
    ITEM_OAK_LOG,
    ITEM_OAK_LEAVES,
    ITEM_GRAVEL,
    ITEM_COBBLESTONE,
    ITEM_OAK_PLANKS,
    ITEM_COAL_ORE,
    ITEM_IRON_ORE,
    ITEM_DIAMOND_ORE,
    ITEM_COAL,
    ITEM_DIAMOND,
    ITEM_FLOWER_DANDELION,
    ITEM_FLOWER_HOUSTONIA,
    ITEM_FLOWER_OXEYE_DAISY,
    ITEM_FLOWER_ALLIUM,
    ITEM_GRASS,
    ITEM_TYPES
} item_types;

typedef struct {
    b8 placable;
    char *tex_path;
} item_data_t;

typedef struct {
    vec2f pos;
    vec2f size;
    vec2f collider_pos;
    vec2f collider_size;
    vec2f vel;
    f32 gravity;
    b8 ground;
    f32 max_fall_speed;
    f32 timer;
    item_types type;
} item_drop;
VEC_DECL(item_drop, vec_item_drop)

void item_drop_item(item_drop *drop, item_types type, vec2f pos);
void item_update_drop(item_drop *drop, chunk *chunks);
void item_draw_drop(item_drop *drop, texture *item_textures);
