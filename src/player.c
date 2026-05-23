#include "player.h"
#include "chunk.h"

void draw_player(player_t *player) {
    draw_rect(player->pos, player->size, RED, 0);
}

void handle_controls(player_t *player, chunk *chunks) {
    get_cam_2D()->pos = VEC2F(player->pos.x - (get_screen_width() * 0.5f),
                              player->pos.y - (get_screen_height() * 0.5f));
    if (is_key_down(KEY_A)) {
        player->vel.x = -player->move_speed;
    } else if (is_key_down(KEY_D)) {
        player->vel.x = player->move_speed;
    } else {
        player->vel.x = 0;
    }
    if (is_key_down(KEY_SPACE) && player->ground) {
        player->vel.y = -player->jmp_force;
        player->ground = false;
    }
    player->vel.y += player->gravity * get_dt();
    if (player->vel.y > player->max_fall_speed) {
        player->vel.y = player->max_fall_speed;
    }

    if (is_key_down(KEY_ESCAPE)) {
        destroy_window();
    }

    if (is_key_down(KEY_G)) {
        player->move_speed += player->move_speed * 10 * get_dt();
    }
    if (is_key_down(KEY_B)) {
        player->move_speed -= player->move_speed * 10 * get_dt();
    }
    player->move_speed = CPM_CLAMP(player->move_speed, 10, 1000000);

    if (is_key_down(KEY_H)) {
        get_cam_2D()->zoom += 2 * get_dt();
    }
    if (is_key_down(KEY_N)) {
        get_cam_2D()->zoom -= 2 * get_dt();
    }
    get_cam_2D()->zoom = CPM_CLAMP(get_cam_2D()->zoom, 0.01f, 10);

    if (is_mouse_down(MOUSE_BUTTON_LEFT)) {
        vec2f mouse_pos = get_screen_to_world_2D(get_mouse_pos());
        if (mouse_pos.x < 0 ||
            mouse_pos.x > MAP_SIZE * CHUNK_SIZE * BLOCK_SIZE) {
            return;
        }
        u32 idx = (u32)mouse_pos.x / (CHUNK_SIZE * BLOCK_SIZE);
        for (i32 i = -1; i < 2; i++) {
            if (idx == 0 && i == -1) {
                continue;
            }
            if (idx == MAP_SIZE - 1 && i == 1) {
                continue;
            }
            tilemap_delete_tile(
                &chunks[idx + i].tiles,
                VEC2F((i32)mouse_pos.x - ((i32)mouse_pos.x % BLOCK_SIZE),
                      (i32)mouse_pos.y - ((i32)mouse_pos.y % BLOCK_SIZE)));
            tilemap_check_collidable_tiles(&chunks[idx + i].tiles,
                                           VEC2F(BLOCK_SIZE, BLOCK_SIZE));
            tilemap_delete_tile(
                &chunks[idx + i].tiles_passable,
                VEC2F((i32)mouse_pos.x - ((i32)mouse_pos.x % BLOCK_SIZE),
                      (i32)mouse_pos.y - ((i32)mouse_pos.y % BLOCK_SIZE)));
        }
    }
}

void move_and_collide(player_t *player, chunk *chunks) {
    player->pos.x += player->vel.x * get_dt();

    i32 idx = (i32)player->pos.x / (CHUNK_SIZE * BLOCK_SIZE);

    if (idx < 0 || idx >= MAP_SIZE) {
        return;
    }

    for (i32 i = -1; i < 2; i++) {
        if (idx == 0 && i == -1) {
            continue;
        }
        if (idx == MAP_SIZE - 1 && i == 1) {
            continue;
        }
        for (u32 t = 0; t < chunks[idx + i].tiles.renderer.count / 6; t++) {
            if (!chunks[idx + i].tiles.renderer.collidable[t]) {
                continue;
            }
            vec2f tile_pos =
                VEC2F(chunks[idx + i].tiles.renderer.vertices[(u64)t * 6].x,
                      chunks[idx + i].tiles.renderer.vertices[(u64)t * 6].y);
            if (player->pos.y + player->size.y <= tile_pos.y) {
                continue;
            }
            if (player->pos.y >= tile_pos.y + BLOCK_SIZE) {
                continue;
            }
            rect_collider player_collider = {.pos = player->pos,
                                             .size = player->size};
            rect_collider tile_collider = {
                .pos = tile_pos, .size = VEC2F(BLOCK_SIZE, BLOCK_SIZE)};

            if (check_collision_rects(player_collider, tile_collider)) {
                if (player->vel.x > 0) {
                    player->pos.x = tile_pos.x - player->size.x;
                } else if (player->vel.x < 0) {
                    player->pos.x = tile_pos.x + BLOCK_SIZE;
                }
                player->vel.x = 0;
            }
        }
    }
    player->pos.y += player->vel.y * get_dt();
    player->ground = false;
    for (i32 i = -1; i < 2; i++) {
        if (idx == 0 && i == -1) {
            continue;
        }
        if (idx == MAP_SIZE - 1 && i == 1) {
            continue;
        }
        for (u32 t = 0; t < chunks[idx + i].tiles.renderer.count / 6; t++) {
            if (!chunks[idx + i].tiles.renderer.collidable[t]) {
                continue;
            }
            vec2f tile_pos =
                VEC2F(chunks[idx + i].tiles.renderer.vertices[(u64)t * 6].x,
                      chunks[idx + i].tiles.renderer.vertices[(u64)t * 6].y);
            if (player->pos.x + player->size.x <= tile_pos.x) {
                continue;
            }
            if (player->pos.x >= tile_pos.x + BLOCK_SIZE) {
                continue;
            }
            rect_collider player_collider = {.pos = player->pos,
                                             .size = player->size};
            rect_collider tile_collider = {
                .pos = tile_pos, .size = VEC2F(BLOCK_SIZE, BLOCK_SIZE)};

            if (check_collision_rects(player_collider, tile_collider)) {
                if (player->vel.y > 0) {
                    player->pos.y = tile_pos.y - player->size.y;
                    player->ground = true;
                } else if (player->vel.y < 0) {
                    player->pos.y = tile_pos.y + BLOCK_SIZE;
                }
                player->vel.y = 0;
            }
        }
    }
}

void set_spawn_point(player_t *player, fnl_state *terrain) {
    f32 noise =
        (fnlGetNoise2D(terrain, (f32)((u32)(0 * CHUNK_SIZE)), 0) + 1.0f) * 0.5f;
    u32 height =
        MIN_TERRAIN_HEIGHT + (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));
    player->pos.y = (f32)(MAX_CHUNK_HEIGHT - height) * BLOCK_SIZE;
}
