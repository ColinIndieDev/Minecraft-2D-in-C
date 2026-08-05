#pragma once

#include "chunk.h"
#include "items.h"

#define PLAYER_BASE_SPEED 250.0f
#define MAX_STACK_SIZE 64
#define MINE_AND_PLACE_RANGE 5

#define UV_EPSILON 0.01f

#define ICON_PIXEL_SIZE 9
#define ICON_HEART_BG VEC2F(0, 0)
#define ICON_HEART VEC2F(1, 0)
#define ICON_HEART_HALF VEC2F(2, 0)
#define ICON_HUNGER_BG VEC2F(0, 3)
#define ICON_HUNGER VEC2F(1, 3)
#define ICON_HUNGER_HALF VEC2F(2, 3)

typedef struct {
    item_type_t item;
    uint32_t count;
} slot_t;

typedef struct {
    slot_t slots[27];
    slot_t hotbar[9];
    uint32_t hotbar_selected;
    bool enabled;
} inventory_t;

typedef struct {
    vec2f pos;
    vec2f size;
    vec2f vel;
    float jmp_force;
    float gravity;
    float move_speed;
    float max_fall_speed;
    bool ground;
} physical_attribs_t;

typedef struct {
    tilemap icons_bg;
    tilemap icons;
    float health;
    float hunger;
} stats_t;

typedef struct {
    vec2f block;
    float block_dt;
    float timer;
} mining_t;

typedef struct {
    stats_t stats;
    inventory_t inventory;
    physical_attribs_t attribs;
    mining_t mining;
} player_t;

void player_init();
void player_update(player_t *player, chunk_entry *chunks, block_data_t *block_data, item_drop_t **drops, 
                   uint32_t left_most_chunk, uint32_t right_most_chunk, texture *inventory, texture *item_textures, texture *hotbar_arrow);
uint32_t player_set_spawn_point(player_t *player, fnl_state *terrain);
void player_draw(player_t *player);
void player_draw_inventory(player_t *player, texture *inventory, texture *item_textures, texture *hotbar_arrow, font *f);
void player_draw_ui(player_t *player, texture *hotbar, texture *hotbar_arrow, texture *item_textures, font *f);
