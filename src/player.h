#pragma once

#include "chunk.h"
#include "items.h"

#define PLAYER_BASE_SPEED 250.0f
#define MAX_STACK_SIZE 64
#define MINE_AND_PLACE_RANGE 5

#define UV_EPSILON 0.001f

#define ICON_PIXEL_SIZE 9
#define ICON_HEART_BG VEC2F(0, 0)
#define ICON_HEART VEC2F(1, 0)
#define ICON_HEART_HALF VEC2F(2, 0)
#define ICON_HUNGER_BG VEC2F(0, 3)
#define ICON_HUNGER VEC2F(1, 3)
#define ICON_HUNGER_HALF VEC2F(2, 3)

typedef struct {
    item_types item;
    u32 count;
} slot;

typedef struct {
    tilemap status_icons_bg;
    tilemap status_icons;
    slot hotbar[9];
    vec2f pos;
    vec2f size;
    vec2f vel;
    vec2f block_mining;
    u32 hotbar_selected;
    f32 jmp_force;
    f32 gravity;
    f32 move_speed;
    f32 max_fall_speed;
    f32 block_mining_dt;
    f32 block_mining_timer;
    f32 health;
    f32 hunger;
    b8 ground;
    b8 in_inventory;
} player_t;

void player_update(player_t *player, chunk *chunks, block_data_t *block_data,
                   vec_item_drop *drops);
void player_set_spawn_point(player_t *player, fnl_state *terrain);
void player_draw(player_t *player);
void player_draw_inventory(player_t *player, texture *inventory, texture *item_textures, texture *hotbar_arrow, font *f);
void player_draw_gui(player_t *player, texture *hotbar, texture *hotbar_arrow,
                     texture *item_textures, font *f);
