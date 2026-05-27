#pragma once

#include "chunk.h"

typedef struct {
    vec2f pos;
    vec2f size;
    vec2f vel;
    b8 ground;
    f32 jmp_force;
    f32 gravity;
    f32 move_speed;
    f32 max_fall_speed;
    vec2f block_mining;
    f32 block_mining_dt;
    f32 block_mining_timer;
} player_t;

i32 get_block_type_id(vec2f uv, vec2f *block_types_uv);
void handle_controls(player_t *player, chunk *chunks, vec2f *block_types_uv, f32 *block_types_mining_dt);
void move_and_collide(player_t *player, chunk *chunks);
void set_spawn_point(player_t *player, fnl_state *terrain);
void draw_player(player_t *player);
