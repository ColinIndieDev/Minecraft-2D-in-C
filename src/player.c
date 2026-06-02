#include "player.h"
#include "blocks.h"
#include "chunk.h"
#include "items.h"

#ifndef __EMSCRIPTEN__
#include <cpl/cpl.h>
#else
#include "../external/cpl.h"
#endif

// {{{ Item-ID <---> Block-ID Functions

i32 player_item_id_to_block_id(i32 item_id) {
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
    case ITEM_FLOWER_DANDELION:
        return BLOCK_FLOWER_DANDELION;
        break;
    case ITEM_FLOWER_HOUSTONIA:
        return BLOCK_FLOWER_HOUSTONIA;
        break;
    case ITEM_FLOWER_OXEYE_DAISY:
        return BLOCK_FLOWER_OXEYE_DAISY;
        break;
    case ITEM_FLOWER_ALLIUM:
        return BLOCK_FLOWER_ALLIUM;
        break;
    case ITEM_GRASS:
        return BLOCK_GRASS;
        break;
    default:
        return -1;
    }
    return -1;
}

i32 player_block_id_to_item_id(i32 block_id) {
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
    case BLOCK_FLOWER_DANDELION:
        return ITEM_FLOWER_DANDELION;
        break;
    case BLOCK_FLOWER_HOUSTONIA:
        return ITEM_FLOWER_HOUSTONIA;
        break;
    case BLOCK_FLOWER_OXEYE_DAISY:
        return ITEM_FLOWER_OXEYE_DAISY;
        break;
    case BLOCK_FLOWER_ALLIUM:
        return ITEM_FLOWER_ALLIUM;
        break;
    case BLOCK_GRASS:
        return ITEM_GRASS;
        break;
    default:
        return -1;
    }
    return -1;
}

// }}}

void player_draw(player_t *player) {
    draw_rect(player->pos, player->size, RED, 0);
}

// IMPORTANT! if tilemap texture gets bigger, it is more vulnerable to precision
i32 player_get_block_type_id(block_data_t *block_data, vec2f uv) {
    for (i32 i = 0; i < BLOCK_TYPES; i++) {
        if (CPM_ABS(uv.x - block_data[i].uv.x) < UV_EPSILON &&
            CPM_ABS(uv.y - block_data[i].uv.y) < UV_EPSILON) {
            return i;
        }
    }
    return -1;
}

// {{{ Update Helper Functions

// {{{ Controls Helper Functions

void player_handle_movement(player_t *player) {
    if (!player->in_inventory) {
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
    } else {
        player->vel.x = 0;
    }
    player->vel.y += player->gravity * get_dt();
    if (player->vel.y > player->max_fall_speed) {
        player->vel.y = player->max_fall_speed;
    }
    if (!player->in_inventory) {
        if (is_key_down(KEY_LEFT_SHIFT)) {
            player->move_speed = PLAYER_BASE_SPEED * 2;
        } else {
            player->move_speed = PLAYER_BASE_SPEED;
        }
    }
    player->move_speed = CPM_CLAMP(player->move_speed, 10, 1000000);
}

vec2f player_raycast_hit_tile(chunk *chunks, block_data_t *block_data,
                              vec2f origin, vec2f d, f32 max_dist) {
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
            vec2f uv =
                tilemap_get_tile_uv(&chunks[chunk_idx].tiles_passable, tile);
            vec2f water_uv = block_data[BLOCK_WATER].uv;
            b8 is_water = CPM_ABS(uv.x - water_uv.x) < UV_EPSILON &&
                          CPM_ABS(uv.y - water_uv.y) < UV_EPSILON;
            if (tilemap_tile_exists(&chunks[chunk_idx].tiles, tile) ||
                (tilemap_tile_exists(&chunks[chunk_idx].tiles_passable, tile) &&
                 !is_water)) {
                return tile;
            }
        }
    }

    return VEC2F(-1, -1);
}

b8 player_neighbor_blocks_exist(chunk *chunks, vec2f mouse_pos_tilemap,
                                u32 idx) {
    vec2f neighbor_blocks[4] = {
        {mouse_pos_tilemap.x + BLOCK_SIZE, mouse_pos_tilemap.y},
        {mouse_pos_tilemap.x - BLOCK_SIZE, mouse_pos_tilemap.y},
        {mouse_pos_tilemap.x, mouse_pos_tilemap.y + BLOCK_SIZE},
        {mouse_pos_tilemap.x, mouse_pos_tilemap.y - BLOCK_SIZE}};
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
                return true;
                break;
            }
        }
    }
    return false;
}

void player_handle_block_placing(player_t *player, chunk *chunks,
                                 block_data_t *block_data, vec2f mouse_pos,
                                 vec2f mouse_pos_tilemap) {
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

        i32 block_id = player_item_id_to_block_id(
            player->hotbar[player->hotbar_selected].item);
        if (!check_collision_rects(player_collider, tile_collider) &&
            !tilemap_tile_exists(&chunks[idx].tiles, mouse_pos_tilemap) &&
            player_neighbor_blocks_exist(chunks, mouse_pos_tilemap, idx) &&
            block_id != -1 &&
            player->hotbar[player->hotbar_selected].count > 0) {
            vec2f uv;
            if (tilemap_tile_exists(&chunks[idx].tiles_passable,
                                    mouse_pos_tilemap)) {
                uv = tilemap_get_tile_uv(&chunks[idx].tiles_passable,
                                         mouse_pos_tilemap);
                vec2f water_uv = block_data[BLOCK_WATER].uv;
                b8 is_water = CPM_ABS(uv.x - water_uv.x) < UV_EPSILON &&
                              CPM_ABS(uv.y - water_uv.y) < UV_EPSILON;
                if (is_water && !block_data[block_id].passable) {
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

void player_handle_block_breaking(player_t *player, chunk *chunks,
                                  block_data_t *block_data,
                                  vec_item_drop *drops, vec2f mouse_pos,
                                  vec2f mouse_pos_tilemap) {
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
        if (!vec2f_cmp(player_raycast_hit_tile(
                           chunks, block_data, player->pos, ray_dir,
                           MINE_AND_PLACE_RANGE * BLOCK_SIZE),
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
        i32 block_id = player_get_block_type_id(block_data, uv);

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
            i32 item_id = player_block_id_to_item_id(block_id);
            if (item_id != -1 && item_id != ITEM_OAK_LEAVES &&
                item_id != ITEM_GRASS) {
                vec_item_drop_push_back(drops, (item_drop){});
                if (block_id == BLOCK_GRASS_BLOCK) {
                    item_drop_item(vec_item_drop_back(drops), ITEM_DIRT,
                                   mouse_pos);
                } else if (block_id == BLOCK_STONE) {
                    item_drop_item(vec_item_drop_back(drops), ITEM_COBBLESTONE,
                                   mouse_pos);
                } else if (block_id == BLOCK_COAL_ORE) {
                    item_drop_item(vec_item_drop_back(drops), ITEM_COAL,
                                   mouse_pos);
                } else if (block_id == BLOCK_DIAMOND_ORE) {
                    item_drop_item(vec_item_drop_back(drops), ITEM_DIAMOND,
                                   mouse_pos);
                } else {
                    item_drop_item(vec_item_drop_back(drops), item_id,
                                   mouse_pos);
                }
            }

            if (tilemap_tile_exists(&chunks[idx].tiles_passable,
                                    mouse_pos_tilemap)) {
                tilemap_delete_tile(&chunks[idx].tiles_passable,
                                    mouse_pos_tilemap);
            } else if (tilemap_tile_exists(&chunks[idx].tiles,
                                           mouse_pos_tilemap)) {
                tilemap_delete_tile(&chunks[idx].tiles, mouse_pos_tilemap);
                tilemap_check_collidable_tiles(&chunks[idx].tiles,
                                               VEC2F(BLOCK_SIZE, BLOCK_SIZE));
            }

            player->block_mining = VEC2F(-1, -1);
            player->block_mining_dt = 0.0f;
            player->block_mining_timer = 0.0f;
        }
    }
}

void player_handle_item_drops(player_t *player, chunk *chunks,
                              vec_item_drop *drops, vec2f mouse_pos,
                              vec2f mouse_pos_tilemap) {
    if (is_key_pressed(KEY_Q)) {
        vec_item_drop_push_back(drops, (item_drop){});
        item_types type = player->hotbar[player->hotbar_selected].item;
        u32 idx = (u32)mouse_pos.x / (CHUNK_SIZE * BLOCK_SIZE);
        if (type != ITEM_NONE &&
            player->hotbar[player->hotbar_selected].count > 0 &&
            !tilemap_tile_exists(&chunks[idx].tiles, mouse_pos_tilemap)) {
            item_drop_item(vec_item_drop_back(drops), type, mouse_pos);
            player->hotbar[player->hotbar_selected].count--;
            if (player->hotbar[player->hotbar_selected].count == 0) {
                player->hotbar[player->hotbar_selected].item = ITEM_NONE;
            }
        }
    }

    {
        u32 w = 0;
        for (u32 i = 0; i < drops->size; i++) {
            item_drop drop = drops->data[i];
            rect_collider drop_collider = {.pos = drop.pos, .size = drop.size};
            rect_collider player_collider = {.pos = player->pos,
                                             .size = player->size};

            if (!check_collision_rects(player_collider, drop_collider)) {
                drops->data[w++] = drops->data[i];
            } else {
                b8 item_picked_up = false;
                for (u32 j = 0; j < 9; j++) {
                    item_types type = drop.type;
                    if (player->hotbar[j].item == type &&
                        player->hotbar[j].count < MAX_STACK_SIZE) {
                        player->hotbar[j].count++;
                        item_picked_up = true;
                        break;
                    }
                }
                if (!item_picked_up) {
                    for (u32 j = 0; j < 9; j++) {
                        item_types type = drop.type;
                        if (player->hotbar[j].item == ITEM_NONE &&
                            player->hotbar[j].count == 0) {
                            player->hotbar[j].item = type;
                            player->hotbar[j].count++;
                            item_picked_up = true;
                            break;
                        }
                    }
                }
                if (!item_picked_up) {
                    drops->data[w++] = drops->data[i];
                }
            }
        }
        drops->size = w;
    }

    FOREACH_VEC(item_drop, vec_item_drop, drop, drops) {
        item_update_drop(drop, chunks);
    }
    u32 w = 0;
    for (u32 i = 0; i < drops->size; i++) {
        item_drop drop = drops->data[i];
        if (!(drop.timer + ITEM_DROP_LIFETIME <= get_time())) {
            drops->data[w++] = drops->data[i];
        }
    }
    drops->size = w;
}

void player_handle_hotbar(player_t *player) {
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
}

// }}}

void player_handle_controls(player_t *player, chunk *chunks,
                            block_data_t *block_data, vec_item_drop *drops) {
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

    if (is_key_pressed(KEY_E)) {
        player->in_inventory = !player->in_inventory;
    }

    player_handle_movement(player);
    if (!player->in_inventory) {
        player_handle_hotbar(player);
        vec2f mouse_pos = get_screen_to_world_2D(get_mouse_pos());
        vec2f mouse_pos_tilemap =
            VEC2F((i32)mouse_pos.x - ((i32)mouse_pos.x % BLOCK_SIZE),
                  (i32)mouse_pos.y - ((i32)mouse_pos.y % BLOCK_SIZE));

        player_handle_block_placing(player, chunks, block_data, mouse_pos,
                                    mouse_pos_tilemap);
        player_handle_block_breaking(player, chunks, block_data, drops,
                                     mouse_pos, mouse_pos_tilemap);
        player_handle_item_drops(player, chunks, drops, mouse_pos,
                                 mouse_pos_tilemap);
    }
}

void player_move_and_collide(player_t *player, chunk *chunks) {
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

// }}}

void player_update(player_t *player, chunk *chunks, block_data_t *block_data,
                   vec_item_drop *drops) {
    player_handle_controls(player, chunks, block_data, drops);
    player_move_and_collide(player, chunks);
}

void player_set_spawn_point(player_t *player, fnl_state *terrain) {
    f32 noise =
        (fnlGetNoise2D(terrain, (f32)((u32)(0 * CHUNK_SIZE)), 0) + 1.0f) * 0.5f;
    u32 height =
        MIN_TERRAIN_HEIGHT + (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));
    player->pos.y = (f32)(MAX_CHUNK_HEIGHT - height) * BLOCK_SIZE;
}

// {{{ GUI

void player_draw_inventory(player_t *player, texture *inventory,
                           texture *item_textures, texture *hotbar_arrow,
                           font *f) {
    if (player->in_inventory) {
        begin_draw(SHAPE_2D_UNLIT, false);
        draw_rect(VEC2F(0, 0), get_screen_size(), RGBA(0, 0, 0, 125), 0);
        begin_draw(TEXTURE_2D_UNLIT, false);
        vec2f size = VEC2F(inventory->size.x * 4, inventory->size.y * 4);
        vec2f pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f),
                          (get_screen_height() * 0.5f) - (size.y * 0.5f));
        draw_texture2D(inventory, pos, size, WHITE, 0);

        vec2f first_hotbar_slot =
            VEC2F(pos.x + (4 * 4), pos.y + size.y - (24 * 4));
        for (u32 i = 0; i < 9; i++) {
            if (player->hotbar[i].count == 0 ||
                player->hotbar[i].item == ITEM_NONE) {
                continue;
            }
            vec2f arrow_size =
                VEC2F(hotbar_arrow->size.x * 4, hotbar_arrow->size.y * 4);
            vec2f item_size =
                VEC2F(item_textures[player->hotbar[i].item].size.x * 4,
                      item_textures[player->hotbar[i].item].size.y * 4);
            vec2f slot_pos = VEC2F(first_hotbar_slot.x + (arrow_size.x * 0.5f) -
                                       (item_size.x * 0.5f) + ((18 * 4) * i),
                                   first_hotbar_slot.y);
            draw_texture2D(&item_textures[player->hotbar[i].item], slot_pos,
                           item_size, WHITE, 0);
        }

        begin_draw(TEXT, false);
        for (u32 i = 0; i < 9; i++) {
            vec2f arrow_size =
                VEC2F(hotbar_arrow->size.x * 4, hotbar_arrow->size.y * 4);
            vec2f item_size =
                VEC2F(item_textures[player->hotbar[i].item].size.x * 4,
                      item_textures[player->hotbar[i].item].size.y * 4);
            f32 offset_y = 20.0f;
            vec2f slot_pos = VEC2F(first_hotbar_slot.x + (arrow_size.x * 0.5f) -
                                       (item_size.x * 0.5f) + ((18 * 4) * i),
                                   first_hotbar_slot.y);
            if (player->hotbar[i].count <= 1 ||
                player->hotbar[i].item == ITEM_NONE) {
                continue;
            }

            char number[3];
            snprintf(number, 3, "%d", player->hotbar[i].count);
            draw_text_shadow(f, number,
                             VEC2F(slot_pos.x + arrow_size.x - (12 * 4),
                                   slot_pos.y + arrow_size.y - (12 * 4)),
                             0.7f, WHITE, VEC2F(4, 4), BLACK);
        }
    }
}

void player_draw_gui(player_t *player, texture *hotbar, texture *hotbar_arrow,
                     texture *item_textures, font *f) {
    if (!player->in_inventory) {
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
            vec2f slot_pos =
                VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f) +
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
                      slot_pos.y + (arrow_size.y * 0.5f) -
                          (item_size.y * 0.5f)),
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
            tilemap_add_tile(&player->status_icons_bg,
                             VEC2F(last_heart_pos.x + (ICON_PIXEL_SIZE * 4) +
                                       ((ICON_PIXEL_SIZE - 1) * 4 * i) +
                                       hunger_bar_offset,
                                   last_heart_pos.y),
                             VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4),
                             ICON_HUNGER_BG);
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
                VEC2F(hotbar_pos.x + ((ICON_PIXEL_SIZE) * 4 *
                                      (u32)(player->health * 0.5f)),
                      hotbar_pos.y - (ICON_PIXEL_SIZE * 4) - icon_offset),
                VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4),
                ICON_HEART_HALF);
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
            vec2f slot_pos =
                VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f) +
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
}

// }}}
