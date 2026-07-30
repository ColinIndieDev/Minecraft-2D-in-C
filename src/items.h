#pragma once

#include "chunk.h"

#include <cpstd/hashmap.h>
#include <cpl/cpl.h>

#define EXTERN_ITEMS_H_VARIABLES                \
                                                \
extern item_data_t item_data[ITEM_TYPE_T_SIZE];

#define ITEM_DROP_LIFETIME (5 * 60)
#define MAX_ANIM_OFFSET 10
#define ITEM_TEX_PATH "assets/images/items/"

typedef enum : uint8_t {
    ITEM_NONE = 0,
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
    ITEM_TYPE_T_SIZE
} item_type_t;

typedef struct {
    char *tex_path;
    bool placable;
} item_data_t;

typedef struct {
    vec2f pos;
    vec2f size;
    vec2f collider_pos;
    vec2f collider_size;
    vec2f vel;
    float gravity;
    float max_fall_speed;
    float timer;
    item_type_t type;
    bool ground;
} item_drop_t;

HM_CREATE_ENTRY(uint32_t, chunk_t *, chunk_entry)

void item_drop_item(item_drop_t *drop, item_type_t type, vec2f pos);
void item_update_drop(item_drop_t *drop, chunk_entry *chunks);
void item_draw_drop(item_drop_t *drop, texture *item_textures);
