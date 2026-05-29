#include "player.h"
#include "blocks.h"
#include "chunk.h"
#include "items.h"

#ifndef __EMSCRIPTEN__
#include <cpl/cpl.h>
#else
#include "../cpstd/cpmath.h"
#include "../external/cpl.h"
#endif

void draw_player(player_t *player) {
    draw_rect(player->pos, player->size, RED, 0);
}

i32 get_block_type_id(vec2f uv, block_data_t *block_data) {
    for (i32 i = 0; i < BLOCK_TYPES; i++) {
        if (vec2f_cmp(uv, block_data[i].uv)) {
            return i;
        }
    }
    return -1;
}

void handle_movement(player_t *player) {
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

    if (is_key_down(KEY_LEFT_SHIFT)) {
        player->move_speed = PLAYER_BASE_SPEED * 2;
    } else {
        player->move_speed = PLAYER_BASE_SPEED;
    }
    player->move_speed = CPM_CLAMP(player->move_speed, 10, 1000000);
}

i32 item_id_to_block_id(i32 item_id) {
    switch (item_id) {
    case ITEM_GRASS_BLOCK:
        return BLOCK_GRASS_BLOCK;
        break;
    case ITEM_DIRT:
        return BLOCK_DIRT;
        break;
    case ITEM_STONE:
        return BLOCK_STONE;
        break;
    case ITEM_SAND:
        return BLOCK_SAND;
        break;
    case ITEM_BEDROCK:
        return BLOCK_BEDROCK;
        break;
    case ITEM_FLOWER_ROSE:
        return BLOCK_FLOWER_ROSE;
        break;
    case ITEM_SUGAR_CANE:
        return BLOCK_SUGAR_CANE;
        break;
    case ITEM_OAK_LOG:
        return BLOCK_OAK_LOG;
        break;
    case ITEM_OAK_LEAVES:
        return BLOCK_OAK_LEAVES;
        break;
    case ITEM_GRAVEL:
        return BLOCK_GRAVEL;
        break;
    case ITEM_COBBLESTONE:
        return BLOCK_COBBLESTONE;
        break;
    case ITEM_OAK_PLANKS:
        return BLOCK_OAK_PLANKS;
        break;
    case ITEM_COAL_ORE:
        return BLOCK_COAL_ORE;
        break;
    case ITEM_IRON_ORE:
        return BLOCK_IRON_ORE;
        break;
    case ITEM_DIAMOND_ORE:
        return BLOCK_DIAMOND_ORE;
        break;
    default:
        return -1;
    }
    return -1;
}

i32 block_id_to_item_id(i32 block_id) {
    switch (block_id) {
    case BLOCK_GRASS_BLOCK:
        return ITEM_GRASS_BLOCK;
        break;
    case BLOCK_DIRT:
        return ITEM_DIRT;
        break;
    case BLOCK_STONE:
        return ITEM_STONE;
        break;
    case BLOCK_SAND:
        return ITEM_SAND;
        break;
    case BLOCK_BEDROCK:
        return ITEM_BEDROCK;
        break;
    case BLOCK_FLOWER_ROSE:
        return ITEM_FLOWER_ROSE;
        break;
    case BLOCK_SUGAR_CANE:
        return ITEM_SUGAR_CANE;
        break;
    case BLOCK_OAK_LOG:
        return ITEM_OAK_LOG;
        break;
    case BLOCK_OAK_LEAVES:
        return ITEM_OAK_LEAVES;
        break;
    case BLOCK_GRAVEL:
        return ITEM_GRAVEL;
        break;
    case BLOCK_COBBLESTONE:
        return ITEM_COBBLESTONE;
        break;
    case BLOCK_OAK_PLANKS:
        return ITEM_OAK_PLANKS;
        break;
    case BLOCK_COAL_ORE:
        return ITEM_COAL_ORE;
        break;
    case BLOCK_IRON_ORE:
        return ITEM_IRON_ORE;
        break;
    case BLOCK_DIAMOND_ORE:
        return ITEM_DIAMOND_ORE;
        break;
    default:
        return -1;
    }
    return -1;
}

vec2f raycast_hit_tile(chunk *chunks, vec2f origin, vec2f d, f32 max_dist,
                       block_data_t *block_data) {
    vec2f dir = vec2f_norm(d);
    vec2f tile = VEC2F(cpm_floorf(origin.x / BLOCK_SIZE) * BLOCK_SIZE,
                       cpm_floorf(origin.y / BLOCK_SIZE) * BLOCK_SIZE);
    vec2f d_dist =
        VEC2F(CPM_ABS(BLOCK_SIZE / dir.x), CPM_ABS(BLOCK_SIZE / dir.y));
    vec2f step = VEC2F(0, 0);
    vec2f side_dist = VEC2F(0, 0);
    if (dir.x < 0) {
        step.x = -BLOCK_SIZE;
        side_dist.x = (origin.x - tile.x) * (d_dist.x / BLOCK_SIZE);
    } else {
        step.x = BLOCK_SIZE;
        side_dist.x =
            (tile.x + BLOCK_SIZE - origin.x) * (d_dist.x / BLOCK_SIZE);
    }
    if (dir.y < 0) {
        step.y = -BLOCK_SIZE;
        side_dist.y = (origin.y - tile.y) * (d_dist.y / BLOCK_SIZE);
    } else {
        step.y = BLOCK_SIZE;
        side_dist.y =
            (tile.y + BLOCK_SIZE - origin.y) * (d_dist.y / BLOCK_SIZE);
    }
    f32 traveled = 0.0f;
    while (traveled < max_dist) {
        if (side_dist.x < side_dist.y) {
            traveled = side_dist.x;
            side_dist.x += d_dist.x;
            tile.x += step.x;
        } else {
            traveled = side_dist.y;
            side_dist.y += d_dist.y;
            tile.y += step.y;
        }
        i32 chunk_idx = (i32)tile.x / (CHUNK_SIZE * BLOCK_SIZE);

        if (tile.x < 0) {
            chunk_idx = (((i32)tile.x + 1) / (CHUNK_SIZE * BLOCK_SIZE)) - 1;
        }

        if (chunk_idx >= 0 && chunk_idx < MAP_SIZE) {
            if (tilemap_tile_exists(&chunks[chunk_idx].tiles, tile) ||
                (tilemap_tile_exists(&chunks[chunk_idx].tiles_passable, tile) &&
                 !vec2f_cmp(tilemap_get_tile_uv(
                                &chunks[chunk_idx].tiles_passable, tile),
                            block_data[BLOCK_WATER].uv))) {
                return tile;
            }
        }
    }

    return VEC2F(-1, -1);
}

void handle_block_placing(player_t *player, vec2f mouse_pos,
                          vec2f mouse_pos_tilemap, chunk *chunks,
                          block_data_t *block_data) {
    if (is_mouse_pressed(MOUSE_BUTTON_RIGHT)) {
        if (vec2f_dist(&player->pos, &mouse_pos) >
            MINE_AND_PLACE_RANGE * BLOCK_SIZE) {
            return;
        }
        if (mouse_pos.x < 0 ||
            mouse_pos.x > MAP_SIZE * CHUNK_SIZE * BLOCK_SIZE) {
            return;
        }
        u32 idx = (u32)mouse_pos.x / (CHUNK_SIZE * BLOCK_SIZE);
        rect_collider player_collider = {.pos = player->pos,
                                         .size = player->size};
        rect_collider tile_collider = {.pos = mouse_pos_tilemap,
                                       .size = VEC2F(BLOCK_SIZE, BLOCK_SIZE)};

        vec2f neighbor_blocks[4] = {
            {mouse_pos_tilemap.x + BLOCK_SIZE, mouse_pos_tilemap.y},
            {mouse_pos_tilemap.x - BLOCK_SIZE, mouse_pos_tilemap.y},
            {mouse_pos_tilemap.x, mouse_pos_tilemap.y + BLOCK_SIZE},
            {mouse_pos_tilemap.x, mouse_pos_tilemap.y - BLOCK_SIZE}};
        b8 neighbor_blocks_exist = false;
        for (i32 c = -1; c < 2; c++) {
            if (c == -1 && idx == 0) {
                continue;
            }
            if (c == 1 && idx == MAP_SIZE - 1) {
                continue;
            }
            for (u32 i = 0; i < 4; i++) {
                if (tilemap_tile_exists(&chunks[idx + c].tiles,
                                        neighbor_blocks[i])) {
                    neighbor_blocks_exist = true;
                    break;
                }
            }
        }
        i32 block_id =
            item_id_to_block_id(player->hotbar[player->hotbar_selected].item);
        if (!check_collision_rects(player_collider, tile_collider) &&
            !tilemap_tile_exists(&chunks[idx].tiles, mouse_pos_tilemap) &&
            neighbor_blocks_exist && block_id != -1 &&
            player->hotbar[player->hotbar_selected].count > 0) {
            vec2f uv;
            if (tilemap_tile_exists(&chunks[idx].tiles_passable,
                                    mouse_pos_tilemap)) {
                uv = tilemap_get_tile_uv(&chunks[idx].tiles_passable,
                                         mouse_pos_tilemap);
                if (vec2f_cmp(uv, block_data[BLOCK_WATER].uv) &&
                    !block_data[block_id].passable) {
                    tilemap_delete_tile(&chunks[idx].tiles_passable,
                                        mouse_pos_tilemap);
                    tilemap_add_tile(&chunks[idx].tiles, mouse_pos_tilemap,
                                     VEC2F(BLOCK_SIZE, BLOCK_SIZE),
                                     block_data[block_id].uv);
                    tilemap_check_collidable_tiles(
                        &chunks[idx].tiles, VEC2F(BLOCK_SIZE, BLOCK_SIZE));

                    player->hotbar[player->hotbar_selected].count--;
                    if (player->hotbar[player->hotbar_selected].count == 0) {
                        player->hotbar[player->hotbar_selected].item =
                            ITEM_NONE;
                    }
                }
            } else {
                if (block_data[block_id].passable) {
                    tilemap_add_tile(
                        &chunks[idx].tiles_passable, mouse_pos_tilemap,
                        VEC2F(BLOCK_SIZE, BLOCK_SIZE), block_data[block_id].uv);
                } else {
                    tilemap_add_tile(&chunks[idx].tiles, mouse_pos_tilemap,
                                     VEC2F(BLOCK_SIZE, BLOCK_SIZE),
                                     block_data[block_id].uv);
                    tilemap_check_collidable_tiles(
                        &chunks[idx].tiles, VEC2F(BLOCK_SIZE, BLOCK_SIZE));
                }

                player->hotbar[player->hotbar_selected].count--;
                if (player->hotbar[player->hotbar_selected].count == 0) {
                    player->hotbar[player->hotbar_selected].item = ITEM_NONE;
                }
            }
        }
    }
}

void handle_block_breaking(player_t *player, vec2f mouse_pos,
                           vec2f mouse_pos_tilemap, chunk *chunks,
                           block_data_t *block_data, vec_item_drop *drops) {
    if (is_mouse_released(MOUSE_BUTTON_LEFT)) {
        player->block_mining = VEC2F(-1, -1);
        player->block_mining_dt = 0.0f;
    }
    if (is_mouse_down(MOUSE_BUTTON_LEFT)) {
        if (vec2f_dist(&player->pos, &mouse_pos) >
            MINE_AND_PLACE_RANGE * BLOCK_SIZE) {
            player->block_mining = VEC2F(-1, -1);
            player->block_mining_dt = 0.0f;
            return;
        }
        if (mouse_pos.x < 0 ||
            mouse_pos.x > MAP_SIZE * CHUNK_SIZE * BLOCK_SIZE) {
            return;
        }
        u32 idx = (u32)mouse_pos.x / (CHUNK_SIZE * BLOCK_SIZE);
        vec2f ray_dir =
            VEC2F(mouse_pos.x - player->pos.x, mouse_pos.y - player->pos.y);
        if (!vec2f_cmp(raycast_hit_tile(chunks, player->pos, ray_dir,
                                        MINE_AND_PLACE_RANGE * BLOCK_SIZE,
                                        block_data),
                       mouse_pos_tilemap)) {
            player->block_mining = VEC2F(-1, -1);
            player->block_mining_dt = 0.0f;
            return;
        }
        vec2f uv;
        if (tilemap_tile_exists(&chunks[idx].tiles_passable,
                                mouse_pos_tilemap)) {
            uv = tilemap_get_tile_uv(&chunks[idx].tiles_passable,
                                     mouse_pos_tilemap);
        } else if (tilemap_tile_exists(&chunks[idx].tiles, mouse_pos_tilemap)) {
            uv = tilemap_get_tile_uv(&chunks[idx].tiles, mouse_pos_tilemap);
        }
        i32 block_id = get_block_type_id(uv, block_data);

        if (!vec2f_cmp(mouse_pos_tilemap, player->block_mining)) {
            player->block_mining = mouse_pos_tilemap;
            player->block_mining_timer = get_time();

            if (block_id == -1 || block_data[block_id].unbreakable) {
                player->block_mining = VEC2F(-1, -1);
                player->block_mining_dt = 0.0f;
            } else {
                player->block_mining_dt = block_data[block_id].base_mining_dt;
            }
        } else if (player->block_mining_timer + player->block_mining_dt <=
                       get_time() &&
                   !vec2f_cmp(player->block_mining, VEC2F(-1, -1))) {
            i32 item_id = block_id_to_item_id(block_id);
            if (item_id != -1 && item_id != ITEM_OAK_LEAVES) {
                vec_item_drop_push_back(drops, (item_drop){});
                if (item_id == ITEM_GRASS_BLOCK) {
                    drop_item(vec_item_drop_back(drops), ITEM_DIRT, mouse_pos);
                } else if (item_id == ITEM_STONE) {
                    drop_item(vec_item_drop_back(drops), ITEM_COBBLESTONE,
                              mouse_pos);
                } else {
                    drop_item(vec_item_drop_back(drops), item_id, mouse_pos);
                }
            }

            tilemap_delete_tile(&chunks[idx].tiles, mouse_pos_tilemap);
            tilemap_check_collidable_tiles(&chunks[idx].tiles,
                                           VEC2F(BLOCK_SIZE, BLOCK_SIZE));
            tilemap_delete_tile(&chunks[idx].tiles_passable, mouse_pos_tilemap);
            player->block_mining = VEC2F(-1, -1);
            player->block_mining_dt = 0.0f;
            player->block_mining_timer = 0.0f;
        }
    }
}

void handle_item_drops(vec_item_drop *drops, vec2f mouse_pos,
                       vec2f mouse_pos_tilemap, player_t *player,
                       chunk *chunks) {
    if (is_key_pressed(KEY_Q)) {
        vec_item_drop_push_back(drops, (item_drop){});
        item_types type = player->hotbar[player->hotbar_selected].item;
        u32 idx = (u32)mouse_pos.x / (CHUNK_SIZE * BLOCK_SIZE);
        if (type != ITEM_NONE &&
            player->hotbar[player->hotbar_selected].count > 0 &&
            !tilemap_tile_exists(&chunks[idx].tiles, mouse_pos_tilemap)) {
            drop_item(vec_item_drop_back(drops), type, mouse_pos);
            player->hotbar[player->hotbar_selected].count--;
            if (player->hotbar[player->hotbar_selected].count == 0) {
                player->hotbar[player->hotbar_selected].item = ITEM_NONE;
            }
        }
    }

    // TODO if hotbar is full, dropped item still gets destroyed
    u32 w = 0;
    for (u32 i = 0; i < drops->size; i++) {
        item_drop drop = drops->data[i];
        rect_collider drop_collider = {.pos = drop.pos, .size = drop.size};
        rect_collider player_collider = {.pos = player->pos,
                                         .size = player->size};

        if (!check_collision_rects(player_collider, drop_collider)) {
            drops->data[w++] = drops->data[i];
        } else {
            b8 found_stackable = false;
            for (u32 i = 0; i < 9; i++) {
                item_types type = drop.type;
                if (player->hotbar[i].item == type &&
                    player->hotbar[i].count < MAX_STACK_SIZE) {
                    player->hotbar[i].count++;
                    found_stackable = true;
                    break;
                }
            }
            if (!found_stackable) {
                for (u32 i = 0; i < 9; i++) {
                    item_types type = drop.type;
                    if (player->hotbar[i].item == ITEM_NONE &&
                        player->hotbar[i].count == 0) {
                        player->hotbar[i].item = type;
                        player->hotbar[i].count++;
                        break;
                    }
                }
            }
        }
    }
    drops->size = w;
}

void handle_controls(player_t *player, chunk *chunks, block_data_t *block_data,
                     vec_item_drop *drops) {
    get_cam_2D()->pos = VEC2F(
        player->pos.x - (get_screen_width() * (1 / get_cam_2D()->zoom) * 0.5f),
        player->pos.y -
            (get_screen_height() * (1 / get_cam_2D()->zoom) * 0.5f));
    if (is_key_down(KEY_H)) {
        get_cam_2D()->zoom += 2 * get_dt();
    }
    if (is_key_down(KEY_N)) {
        get_cam_2D()->zoom -= 2 * get_dt();
    }
    get_cam_2D()->zoom = CPM_CLAMP(get_cam_2D()->zoom, 0.01f, 10);

    if (is_key_down(KEY_ESCAPE)) {
        destroy_window();
    }

    if (is_key_down(KEY_1)) {
        player->hotbar_selected = 0;
    }
    if (is_key_down(KEY_2)) {
        player->hotbar_selected = 1;
    }
    if (is_key_down(KEY_3)) {
        player->hotbar_selected = 2;
    }
    if (is_key_down(KEY_4)) {
        player->hotbar_selected = 3;
    }
    if (is_key_down(KEY_5)) {
        player->hotbar_selected = 4;
    }
    if (is_key_down(KEY_6)) {
        player->hotbar_selected = 5;
    }
    if (is_key_down(KEY_7)) {
        player->hotbar_selected = 6;
    }
    if (is_key_down(KEY_8)) {
        player->hotbar_selected = 7;
    }
    if (is_key_down(KEY_9)) {
        player->hotbar_selected = 8;
    }

    handle_movement(player);

    vec2f mouse_pos = get_screen_to_world_2D(get_mouse_pos());
    vec2f mouse_pos_tilemap =
        VEC2F((i32)mouse_pos.x - ((i32)mouse_pos.x % BLOCK_SIZE),
              (i32)mouse_pos.y - ((i32)mouse_pos.y % BLOCK_SIZE));

    handle_block_placing(player, mouse_pos, mouse_pos_tilemap, chunks,
                         block_data);
    handle_block_breaking(player, mouse_pos, mouse_pos_tilemap, chunks,
                          block_data, drops);

    handle_item_drops(drops, mouse_pos, mouse_pos_tilemap, player, chunks);
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
                    player->pos.y = tile_pos.y + BLOCK_SIZE +
                                    0.1f; // Prevent glitching through
                                          // blocks if jumping into them
                                          // below by slightly offsetting
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

void draw_gui(player_t *player, texture *hotbar, texture *hotbar_arrow,
              texture *item_textures, font *f) {
    begin_draw(TEXTURE_2D_UNLIT, false);

    vec2f size = VEC2F(hotbar->size.x * 4, hotbar->size.y * 4);
    f32 offset_y = 20.0f;
    vec2f hotbar_pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f),
                             get_screen_height() - size.y - offset_y);
    draw_texture2D(hotbar, hotbar_pos, size, WHITE, 0);

    vec2f arrow_size =
        VEC2F(hotbar_arrow->size.x * 4, hotbar_arrow->size.y * 4);
    draw_texture2D(
        hotbar_arrow,
        VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f) +
                  ((arrow_size.x - (4 * 4)) * player->hotbar_selected) - 4,
              get_screen_height() - size.y - offset_y - 4),
        arrow_size, WHITE, 0);

    for (u32 i = 0; i < 9; i++) {
        vec2f slot_pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f) +
                                   ((arrow_size.x - (4 * 4)) * i) - 4,
                               get_screen_height() - size.y - offset_y - 4);
        if (player->hotbar[i].count == 0 ||
            player->hotbar[i].item == ITEM_NONE) {
            continue;
        }
        vec2f item_size =
            VEC2F(item_textures[player->hotbar[i].item].size.x * 4,
                  item_textures[player->hotbar[i].item].size.y * 4);
        draw_texture2D(
            &item_textures[player->hotbar[i].item],
            VEC2F(slot_pos.x + (arrow_size.x * 0.5f) - (item_size.x * 0.5f),
                  slot_pos.y + (arrow_size.y * 0.5f) - (item_size.y * 0.5f)),
            item_size, WHITE, 0);
    }

    tilemap_begin_editing(&player->status_icons_bg);
    f32 icon_offset = 10.0f;
    for (u32 i = 0; i < 10; i++) {
        tilemap_add_tile(
            &player->status_icons_bg,
            VEC2F(hotbar_pos.x + ((ICON_PIXEL_SIZE) * 4 * i),
                  hotbar_pos.y - (ICON_PIXEL_SIZE * 4) - icon_offset),
            VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HEART_BG);
    }

    vec2f last_heart_pos =
        VEC2F(hotbar_pos.x + ((ICON_PIXEL_SIZE) * 4 * 9),
              hotbar_pos.y - (ICON_PIXEL_SIZE * 4) - icon_offset);

    f32 hunger_bar_offset = 10.0f * 4;
    for (u32 i = 0; i < 10; i++) {
        tilemap_add_tile(
            &player->status_icons_bg,
            VEC2F(last_heart_pos.x + (ICON_PIXEL_SIZE * 4) +
                      ((ICON_PIXEL_SIZE - 1) * 4 * i) + hunger_bar_offset,
                  last_heart_pos.y),
            VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HUNGER_BG);
    }

    tilemap_begin_editing(&player->status_icons);
    for (u32 i = 0; i < (u32)(player->health * 0.5f); i++) {
        tilemap_add_tile(
            &player->status_icons,
            VEC2F(hotbar_pos.x + ((ICON_PIXEL_SIZE) * 4 * i),
                  hotbar_pos.y - (ICON_PIXEL_SIZE * 4) - icon_offset),
            VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HEART);
    }
    if (cpm_modf(player->health, 2.0f) == 1.0f) {
        tilemap_add_tile(
            &player->status_icons,
            VEC2F(hotbar_pos.x +
                      ((ICON_PIXEL_SIZE) * 4 * (u32)(player->health * 0.5f)),
                  hotbar_pos.y - (ICON_PIXEL_SIZE * 4) - icon_offset),
            VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HEART_HALF);
    }

    for (u32 i = 0; i < (u32)(player->hunger * 0.5f); i++) {
        tilemap_add_tile(
            &player->status_icons,
            VEC2F(last_heart_pos.x + (ICON_PIXEL_SIZE * 4) +
                      ((ICON_PIXEL_SIZE - 1) * 4 * i) + hunger_bar_offset,
                  last_heart_pos.y),
            VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HUNGER);
    }
    if (cpm_modf(player->hunger, 2.0f) == 1.0f) {
        tilemap_add_tile(&player->status_icons,
                         VEC2F(last_heart_pos.x + (ICON_PIXEL_SIZE * 4) +
                                   ((ICON_PIXEL_SIZE - 1) * 4 *
                                    (u32)(player->hunger * 0.5f)) +
                                   hunger_bar_offset,
                               last_heart_pos.y),
                         VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4),
                         ICON_HUNGER_HALF);
    }

    tilemap_draw(&player->status_icons_bg, WHITE);
    tilemap_draw(&player->status_icons, WHITE);

    begin_draw(TEXT, false);

    for (u32 i = 0; i < 9; i++) {
        vec2f slot_pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f) +
                                   ((arrow_size.x - (4 * 4)) * i) - 4,
                               get_screen_height() - size.y - offset_y - 4);
        if (player->hotbar[i].count <= 1 ||
            player->hotbar[i].item == ITEM_NONE) {
            continue;
        }

        char number[3];
        snprintf(number, 3, "%d", player->hotbar[i].count);
        draw_text_shadow(f, number,
                         VEC2F(slot_pos.x + arrow_size.x - (9 * 4),
                               slot_pos.y + arrow_size.y - (9 * 4)),
                         0.7f, WHITE, VEC2F(4, 4), BLACK);
    }
}
