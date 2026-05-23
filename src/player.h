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
} player_t;

void handle_controls(player_t *player, chunk *chunks);
void move_and_collide(player_t *player, chunk *chunks);
void set_spawn_point(player_t *player, fnl_state *terrain);
void draw_player(player_t *player);
