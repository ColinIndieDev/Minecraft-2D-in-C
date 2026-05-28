#pragma once

#ifndef __EMSCRIPTEN__
#include <cpl/cpl.h>
#endif
#ifdef __EMSCRIPTEN__
#include "../cpstd/cpmath.h"
#include "../external/cpl.h"
#endif

#include "chunk.h"

#define ITEM_DROP_LIFETIME (5 * 60)

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
    ITEM_TYPES
} item_types;

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

b8 item_placable(item_types type);
void drop_item(item_drop *drop, vec2f pos, item_types type);
void update_drop(item_drop *drop, chunk *chunks);
void draw_drop(item_drop *drop, texture *item_textures);
