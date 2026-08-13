#pragma once

#include "chunk.h"
#include "items.h"

#define PLAYER_BASE_SPEED 250.0f
#define MAX_STACK_SIZE 64
#define MAX_INVENTORY_SLOTS 27
#define MAX_HOTBAR_SLOTS 9
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
    tilemap_t icons_bg;
    tilemap_t icons;
    float health;
    float hunger;
} stats_t;

typedef struct {
    vec2f block;
    float block_dt;
    float timer;
} mining_t;

mining_t *player_get_mining_properties();
stats_t *player_get_stats_properties();
inventory_t *player_get_inventory_properties();
physical_attribs_t *player_get_attribs_properties();
void player_update(item_drop_t **drops);
uint32_t player_set_spawn_point(fnl_state *terrain);
void player_draw();
void player_draw_inventory();
void player_draw_ui();
