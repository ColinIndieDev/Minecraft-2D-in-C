#include "player.h"
#include "blocks.h"
#include "chunk.h"
#include "items.h"

#include <cpstd/hashmap.h>
#include <cpstd/mathplus.h>
#include <cpstd/vector.h>
#include <cpl/cpl.h>

#pragma region Item and Block ID convertion

int player_item_id_to_block_id(int item_id) {
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

int player_block_id_to_item_id(int block_id) {
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

#pragma endregion

void player_draw(player_t *player) {
    draw_rect(player->attribs.pos, player->attribs.size, RED, 0);
}

// If tilemap texture gets bigger, it is more vulnerable to cumbersome precision
int player_get_block_type_id(block_data_t *block_data, vec2f uv) {
    for (int i = 0; i < BLOCK_TYPE_T_SIZE; i++) {
        if (math_abs(uv.x - block_data[i].uv.x) < UV_EPSILON && math_abs(uv.y - block_data[i].uv.y) < UV_EPSILON) {
            return i;
        }
    }
    return -1;
}

#pragma region player_update() Helper

#pragma region player_handle_controls() Helper

void player_handle_movement(player_t *player) {
    if (!player->inventory.enabled) {
        if (is_key_down(KEY_LETTER_A)) {
            player->attribs.vel.x = -player->attribs.move_speed;
        } else if (is_key_down(KEY_LETTER_D)) {
            player->attribs.vel.x = player->attribs.move_speed;
        } else {
            player->attribs.vel.x = 0;
        }
        if (is_key_down(KEY_SPACE) && player->attribs.ground) {
            player->attribs.vel.y = -player->attribs.jmp_force;
            player->attribs.ground = false;
        }
    } else {
        player->attribs.vel.x = 0;
    }
    player->attribs.vel.y += player->attribs.gravity * get_dt();
    if (player->attribs.vel.y > player->attribs.max_fall_speed) {
        player->attribs.vel.y = player->attribs.max_fall_speed;
    }
    if (!player->inventory.enabled) {
        if (is_key_down(KEY_LEFT_SHIFT)) {
            player->attribs.move_speed = PLAYER_BASE_SPEED * 2;
        } else {
            player->attribs.move_speed = PLAYER_BASE_SPEED;
        }
    }
    player->attribs.move_speed = math_clamp(player->attribs.move_speed, 10, 1000000);
}

vec2f player_raycast_hit_tile(chunk_entry *chunks, block_data_t *block_data, vec2f origin, vec2f d, float max_dist) {
    vec2f dir = vec2f_norm(d);
    vec2f tile = vec2f_float_mul(VEC2F(floorf(origin.x / BLOCK_SIZE), floorf(origin.y / BLOCK_SIZE)), BLOCK_SIZE);
    vec2f d_dist = VEC2F(math_abs(BLOCK_SIZE / dir.x), math_abs(BLOCK_SIZE / dir.y));
    vec2f step = VEC2F(0, 0);
    vec2f side_dist = VEC2F(0, 0);
    if (dir.x < 0) {
        step.x = -BLOCK_SIZE;
        side_dist.x = (origin.x - tile.x) * (d_dist.x / BLOCK_SIZE);
    } else {
        step.x = BLOCK_SIZE;
        side_dist.x = (tile.x + BLOCK_SIZE - origin.x) * (d_dist.x / BLOCK_SIZE);
    }
    if (dir.y < 0) {
        step.y = -BLOCK_SIZE;
        side_dist.y = (origin.y - tile.y) * (d_dist.y / BLOCK_SIZE);
    } else {
        step.y = BLOCK_SIZE;
        side_dist.y = (tile.y + BLOCK_SIZE - origin.y) * (d_dist.y / BLOCK_SIZE);
    }
    float traveled = 0.0f;
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
        int chunk_idx = (int)tile.x / (CHUNK_SIZE * BLOCK_SIZE);

        if (tile.x < 0) {
            chunk_idx = (((int)tile.x + 1) / (CHUNK_SIZE * BLOCK_SIZE)) - 1;
        }

        if (chunk_idx >= 0 && chunk_idx < MAP_SIZE) {
            vec2f uv = tilemap_get_tile_uv(&(*hm_get(chunks, chunk_idx))->tiles_passable, tile);
            vec2f water_uv = block_data[BLOCK_WATER].uv;
            bool is_water = math_abs(uv.x - water_uv.x) < UV_EPSILON && math_abs(uv.y - water_uv.y) < UV_EPSILON;
            if (tilemap_tile_exists(&(*hm_get(chunks, chunk_idx))->tiles, tile) ||
                (tilemap_tile_exists(&(*hm_get(chunks, chunk_idx))->tiles_passable, tile) && !is_water)) {
                return tile;
            }
        }
    }
    return VEC2F(-1, -1);
}

bool player_neighbor_blocks_exist(chunk_entry *chunks, vec2f mouse_pos_tilemap, uint32_t idx, uint32_t left_most_chunk, uint32_t right_most_chunk) {
    vec2f neighbor_blocks[4] = {
        {mouse_pos_tilemap.x + BLOCK_SIZE, mouse_pos_tilemap.y},
        {mouse_pos_tilemap.x - BLOCK_SIZE, mouse_pos_tilemap.y},
        {mouse_pos_tilemap.x, mouse_pos_tilemap.y + BLOCK_SIZE},
        {mouse_pos_tilemap.x, mouse_pos_tilemap.y - BLOCK_SIZE}
    };
    for (int c = -1; c < 2; c++) {
        if (c == -1 && idx == left_most_chunk) {
            continue;
        }
        if (c == 1 && idx == right_most_chunk) {
            continue;
        }
        for (uint32_t i = 0; i < 4; i++) {
            if (tilemap_tile_exists(&(*hm_get(chunks, idx + c))->tiles, neighbor_blocks[i])) {
                return true;
                break;
            }
        }
    }
    return false;
}

#pragma region player_handle_block_placing() Helper

bool player_can_place_block(rect_collider player_collider, rect_collider tile_collider, chunk_entry *chunks, vec2f mouse_pos_tilemap, 
                            uint32_t left_most_chunk, uint32_t right_most_chunk, player_t *player,  uint32_t idx, int block_id) {
    return !check_collision_rects(player_collider, tile_collider) && !tilemap_tile_exists(&(*hm_get(chunks, idx))->tiles, mouse_pos_tilemap) &&
            player_neighbor_blocks_exist(chunks, mouse_pos_tilemap, idx, left_most_chunk, right_most_chunk) && block_id != -1 &&
            player->inventory.hotbar[player->inventory.hotbar_selected].count > 0;
}

#pragma endregion

void player_handle_block_placing(player_t *player, chunk_entry *chunks, block_data_t *block_data, vec2f mouse_pos, vec2f mouse_pos_tilemap,
                                 uint32_t left_most_chunk, uint32_t right_most_chunk) {
    if (is_mouse_pressed(MOUSE_BUTTON_RIGHT)) {
        if (vec2f_dist(player->attribs.pos, mouse_pos) > MINE_AND_PLACE_RANGE * BLOCK_SIZE) {
            return;
        }
        if (mouse_pos.x < (float)left_most_chunk * CHUNK_SIZE * BLOCK_SIZE || mouse_pos.x > (float)(right_most_chunk + 1) * CHUNK_SIZE * BLOCK_SIZE) {
            return;
        }
        uint32_t idx = (uint32_t)mouse_pos.x / (CHUNK_SIZE * BLOCK_SIZE);
        rect_collider player_collider = {
            .pos = player->attribs.pos,
            .size = player->attribs.size
        };
        rect_collider tile_collider = {
            .pos = mouse_pos_tilemap,
            .size = VEC2F(BLOCK_SIZE, BLOCK_SIZE)
        };

        int block_id = player_item_id_to_block_id(player->inventory.hotbar[player->inventory.hotbar_selected].item);
        if (player_can_place_block(player_collider, tile_collider, chunks, mouse_pos_tilemap, 
                                   left_most_chunk, right_most_chunk, player, idx, block_id)) {
            vec2f uv = VEC2F(0, 0);
            if (tilemap_tile_exists(&(*hm_get(chunks, idx))->tiles_passable, mouse_pos_tilemap)) {
                uv = tilemap_get_tile_uv(&(*hm_get(chunks, idx))->tiles_passable, mouse_pos_tilemap);
                vec2f water_uv = block_data[BLOCK_WATER].uv;
                bool is_water = math_abs(uv.x - water_uv.x) < UV_EPSILON && math_abs(uv.y - water_uv.y) < UV_EPSILON;
                if (is_water && !block_data[block_id].passable) {
                    tilemap_delete_tile(&(*hm_get(chunks, idx))->tiles_passable, mouse_pos_tilemap);
                    tilemap_add_tile(&(*hm_get(chunks, idx))->tiles, mouse_pos_tilemap, VEC2F(BLOCK_SIZE, BLOCK_SIZE), block_data[block_id].uv);
                    tilemap_check_collidable_tiles(&(*hm_get(chunks, idx))->tiles, VEC2F(BLOCK_SIZE, BLOCK_SIZE));

                    player->inventory.hotbar[player->inventory.hotbar_selected].count--;
                    if (player->inventory.hotbar[player->inventory.hotbar_selected].count == 0) {
                        player->inventory.hotbar[player->inventory.hotbar_selected].item = ITEM_NONE;
                    }
                }
            } else {
                if (block_data[block_id].passable) {
                    tilemap_add_tile(&(*hm_get(chunks, idx))->tiles_passable, mouse_pos_tilemap, VEC2F(BLOCK_SIZE, BLOCK_SIZE), block_data[block_id].uv);
                } else {
                    tilemap_add_tile(&(*hm_get(chunks, idx))->tiles, mouse_pos_tilemap, VEC2F(BLOCK_SIZE, BLOCK_SIZE), block_data[block_id].uv);
                    tilemap_check_collidable_tiles(&(*hm_get(chunks, idx))->tiles, VEC2F(BLOCK_SIZE, BLOCK_SIZE));
                }

                player->inventory.hotbar[player->inventory.hotbar_selected].count--;
                if (player->inventory.hotbar[player->inventory.hotbar_selected].count == 0) {
                    player->inventory.hotbar[player->inventory.hotbar_selected].item = ITEM_NONE;
                }
            }
        }
    }
}

void player_handle_block_breaking(player_t *player, chunk_entry *chunks, block_data_t *block_data, item_drop_t **drops, 
                                  vec2f mouse_pos, vec2f mouse_pos_tilemap, uint32_t left_most_chunk, uint32_t right_most_chunk) {
    if (is_mouse_released(MOUSE_BUTTON_LEFT)) {
        fprintf(stderr, "Released\n");
        player->mining.block = VEC2F(-1, -1);
        player->mining.block_dt = 0.0f;
    }
    if (is_mouse_down(MOUSE_BUTTON_LEFT)) {
        if (vec2f_dist(player->attribs.pos, mouse_pos) > MINE_AND_PLACE_RANGE * BLOCK_SIZE) {
            fprintf(stderr, "Out of Range\n");
            player->mining.block = VEC2F(-1, -1);
            player->mining.block_dt = 0.0f;
            return;
        }
        if (mouse_pos.x < (float)left_most_chunk * CHUNK_SIZE * BLOCK_SIZE || mouse_pos.x > (float)(right_most_chunk + 1) * CHUNK_SIZE * BLOCK_SIZE) {
            return;
        }
        uint32_t idx = (uint32_t)mouse_pos.x / (CHUNK_SIZE * BLOCK_SIZE);
        vec2f ray_dir = VEC2F(mouse_pos.x - player->attribs.pos.x, mouse_pos.y - player->attribs.pos.y);
        if (!vec2f_cmp(player_raycast_hit_tile(chunks, block_data, player->attribs.pos, ray_dir, MINE_AND_PLACE_RANGE * BLOCK_SIZE), mouse_pos_tilemap)) {
            fprintf(stderr, "Block in way\n");
            player->mining.block = VEC2F(-1, -1);
            player->mining.block_dt = 0.0f;
            return;
        }
        vec2f uv = VEC2F(0, 0);
        if (tilemap_tile_exists(&(*hm_get(chunks, idx))->tiles_passable, mouse_pos_tilemap)) {
            uv = tilemap_get_tile_uv( &(*hm_get(chunks, idx))->tiles_passable, mouse_pos_tilemap);
        } else if (tilemap_tile_exists(&(*hm_get(chunks, idx))->tiles, mouse_pos_tilemap)) {
            uv = tilemap_get_tile_uv(&(*hm_get(chunks, idx))->tiles, mouse_pos_tilemap);
        }
        int block_id = player_get_block_type_id(block_data, uv);

        if (!vec2f_cmp(mouse_pos_tilemap, player->mining.block)) {
            player->mining.block = mouse_pos_tilemap;
            player->mining.timer = get_time();

            if (block_id == -1 || block_data[block_id].unbreakable) {
                fprintf(stderr, "Invalid or unbreakable\n");
                player->mining.block = VEC2F(-1, -1);
                player->mining.block_dt = 0.0f;
            } else {
                player->mining.block_dt = block_data[block_id].base_mining_dt;
            }
        } else if (player->mining.timer + player->mining.block_dt <= get_time() && !vec2f_cmp(player->mining.block, VEC2F(-1, -1))) {
            int item_id = player_block_id_to_item_id(block_id);
            if (item_id != -1 && item_id != ITEM_OAK_LEAVES &&
                item_id != ITEM_GRASS) {
                vec_push(*drops, (item_drop_t){});
                if (block_id == BLOCK_GRASS_BLOCK) {
                    item_drop_item(vec_back(*drops), ITEM_DIRT, mouse_pos);
                } else if (block_id == BLOCK_STONE) {
                    item_drop_item(vec_back(*drops), ITEM_COBBLESTONE, mouse_pos);
                } else if (block_id == BLOCK_COAL_ORE) {
                    item_drop_item(vec_back(*drops), ITEM_COAL, mouse_pos);
                } else if (block_id == BLOCK_DIAMOND_ORE) {
                    item_drop_item(vec_back(*drops), ITEM_DIAMOND, mouse_pos);
                } else {
                    item_drop_item(vec_back(*drops), item_id, mouse_pos);
                }
            }

            if (tilemap_tile_exists(&(*hm_get(chunks, idx))->tiles_passable, mouse_pos_tilemap)) {
                tilemap_delete_tile(&(*hm_get(chunks, idx))->tiles_passable, mouse_pos_tilemap);
            } else if (tilemap_tile_exists(&(*hm_get(chunks, idx))->tiles, mouse_pos_tilemap)) {
                tilemap_delete_tile(&(*hm_get(chunks, idx))->tiles, mouse_pos_tilemap);
                tilemap_check_collidable_tiles(&(*hm_get(chunks, idx))->tiles, VEC2F(BLOCK_SIZE, BLOCK_SIZE));
            }

            player->mining.block = VEC2F(-1, -1);
            player->mining.block_dt = 0.0f;
            player->mining.timer = 0.0f;
        }
    }
}

void player_handle_item_drops(player_t *player, chunk_entry *chunks, item_drop_t **drops, vec2f mouse_pos, vec2f mouse_pos_tilemap, 
                              uint32_t left_most_chunk, uint32_t right_most_chunk) {
    if (is_key_pressed(KEY_LETTER_Q)) {
        vec_push(*drops, (item_drop_t){});
        item_type_t type = player->inventory.hotbar[player->inventory.hotbar_selected].item;
        uint32_t idx = (uint32_t)mouse_pos.x / (CHUNK_SIZE * BLOCK_SIZE);
        if (idx < left_most_chunk || idx > right_most_chunk) {
            return;
        }
        if (type != ITEM_NONE && player->inventory.hotbar[player->inventory.hotbar_selected].count > 0 && 
            !tilemap_tile_exists(&(*hm_get(chunks, idx))->tiles, mouse_pos_tilemap)) {
            item_drop_item(vec_back(*drops), type, mouse_pos);

            player->inventory.hotbar[player->inventory.hotbar_selected].count--;
            if (player->inventory.hotbar[player->inventory.hotbar_selected].count == 0) {
                player->inventory.hotbar[player->inventory.hotbar_selected].item = ITEM_NONE;
            }
        }
    }

    {
        uint32_t w = 0;
        for (uint32_t i = 0; i < vec_size(*drops); i++) {
            item_drop_t drop = (*drops)[i];
            rect_collider drop_collider = {
                .pos = drop.pos, 
                .size = drop.size
            };
            rect_collider player_collider = {
                .pos = player->attribs.pos,
                .size = player->attribs.size
            };

            if (!check_collision_rects(player_collider, drop_collider)) {
                (*drops)[w++] = (*drops)[i];
            } else {
                bool item_picked_up = false;
                for (uint32_t j = 0; j < 9; j++) {
                    item_type_t type = drop.type;
                    if (player->inventory.hotbar[j].item == type &&
                        player->inventory.hotbar[j].count < MAX_STACK_SIZE) {
                        player->inventory.hotbar[j].count++;
                        item_picked_up = true;
                        break;
                    }
                }
                if (!item_picked_up) {
                    for (uint32_t j = 0; j < 9; j++) {
                        item_type_t type = drop.type;
                        if (player->inventory.hotbar[j].item == ITEM_NONE &&
                            player->inventory.hotbar[j].count == 0) {
                            player->inventory.hotbar[j].item = type;
                            player->inventory.hotbar[j].count++;
                            item_picked_up = true;
                            break;
                        }
                    }
                }
                if (!item_picked_up) {
                    (*drops)[w++] = (*drops)[i];
                }
            }
        }
        hm_header(*drops)->size = w;
    }

    foreach_vec(drop, *drops) {
        item_update_drop(drop, chunks);
    }
    vec_erase_if(drop, *drops, drop->timer + ITEM_DROP_LIFETIME <= get_time());
}

void player_handle_hotbar(player_t *player) {
    if (is_key_down(KEY_DIGIT_1)) {
        player->inventory.hotbar_selected = 0;
    }
    if (is_key_down(KEY_DIGIT_2)) {
        player->inventory.hotbar_selected = 1;
    }
    if (is_key_down(KEY_DIGIT_3)) {
        player->inventory.hotbar_selected = 2;
    }
    if (is_key_down(KEY_DIGIT_4)) {
        player->inventory.hotbar_selected = 3;
    }
    if (is_key_down(KEY_DIGIT_5)) {
        player->inventory.hotbar_selected = 4;
    }
    if (is_key_down(KEY_DIGIT_6)) {
        player->inventory.hotbar_selected = 5;
    }
    if (is_key_down(KEY_DIGIT_7)) {
        player->inventory.hotbar_selected = 6;
    }
    if (is_key_down(KEY_DIGIT_8)) {
        player->inventory.hotbar_selected = 7;
    }
    if (is_key_down(KEY_DIGIT_9)) {
        player->inventory.hotbar_selected = 8;
    }
}

#pragma endregion

void player_handle_controls(player_t *player, chunk_entry *chunks, block_data_t *block_data, item_drop_t **drops,
                            uint32_t left_most_chunk, uint32_t right_most_chunk) {
    get_cam_2D()->pos = VEC2F(player->attribs.pos.x - (get_screen_width() * (1 / get_cam_2D()->zoom) * 0.5f),
                              player->attribs.pos.y - (get_screen_height() * (1 / get_cam_2D()->zoom) * 0.5f));
    if (is_key_down(KEY_LETTER_H)) {
        get_cam_2D()->zoom += 2 * get_dt();
    }
    if (is_key_down(KEY_LETTER_N)) {
        get_cam_2D()->zoom -= 2 * get_dt();
    }
    get_cam_2D()->zoom = math_clamp(get_cam_2D()->zoom, 0.01f, 10);

    if (is_key_down(KEY_ESCAPE)) {
        window_destroy();
    }

    if (is_key_pressed(KEY_LETTER_E)) {
        player->inventory.enabled = !player->inventory.enabled;
    }

    player_handle_movement(player);
    if (!player->inventory.enabled) {
        player_handle_hotbar(player);
        vec2f mouse_pos = get_screen_to_world_2D(get_mouse_pos());
        vec2f mouse_pos_tilemap = VEC2F((int)mouse_pos.x - ((int)mouse_pos.x % BLOCK_SIZE), (int)mouse_pos.y - ((int)mouse_pos.y % BLOCK_SIZE));

        player_handle_block_placing(player, chunks, block_data, mouse_pos, mouse_pos_tilemap, left_most_chunk, right_most_chunk);
        player_handle_block_breaking(player, chunks, block_data, drops, mouse_pos, mouse_pos_tilemap, left_most_chunk, right_most_chunk);
        player_handle_item_drops(player, chunks, drops, mouse_pos, mouse_pos_tilemap, left_most_chunk, right_most_chunk);
    }
}

// TODO make move and collide better and not glitchy to get under the ground

void player_move_and_collide(player_t *player, chunk_entry *chunks, uint32_t left_most_chunk, uint32_t right_most_chunk) {
    float dt = get_dt();
    
    player->attribs.pos.x += player->attribs.vel.x * dt;
    int idx = (int)player->attribs.pos.x / (CHUNK_SIZE * BLOCK_SIZE);

    if (idx < (int)left_most_chunk || idx > (int)right_most_chunk) {
        return;
    }

    for (int i = -1; i < 2; i++) {
        int current_idx = idx + i;
        if (current_idx < (int)left_most_chunk || current_idx > (int)right_most_chunk) {
            continue;
        }

        chunk_t **chunk_ptr = hm_get(chunks, current_idx);
        if (!chunk_ptr || !*chunk_ptr || !(*chunk_ptr)->ready) {
            continue; 
        }
        chunk_t *c = *chunk_ptr;

        uint32_t tile_count = c->tiles.renderer.count / 6;
        for (uint32_t t = 0; t < tile_count; t++) {
            if (!c->tiles.renderer.collidable[t]) {
                continue;
            }

            vec2f tile_pos = VEC2F(c->tiles.renderer.vertices[(size_t)t * 6].x, c->tiles.renderer.vertices[(size_t)t * 6].y);

            if (player->attribs.pos.y + player->attribs.size.y <= tile_pos.y || player->attribs.pos.y >= tile_pos.y + BLOCK_SIZE) {
                continue;
            }

            rect_collider player_collider = {
                .pos = player->attribs.pos, 
                .size = player->attribs.size
            };
            rect_collider tile_collider = {
                .pos = tile_pos, 
                .size = VEC2F(BLOCK_SIZE, BLOCK_SIZE)
            };

            if (check_collision_rects(player_collider, tile_collider)) {
                if (player->attribs.vel.x > 0) {
                    player->attribs.pos.x = tile_pos.x - player->attribs.size.x - 0.01f;
                } else if (player->attribs.vel.x < 0) {
                    player->attribs.pos.x = tile_pos.x + BLOCK_SIZE + 0.01f;
                }
                player->attribs.vel.x = 0;
            }
        }
    }

    player->attribs.pos.y += player->attribs.vel.y * dt;
    player->attribs.ground = false;

    idx = (int)player->attribs.pos.x / (CHUNK_SIZE * BLOCK_SIZE);

    for (int i = -1; i < 2; i++) {
        int current_idx = idx + i;
        if (current_idx < (int)left_most_chunk || current_idx > (int)right_most_chunk) {
            continue;
        }

        chunk_t **chunk_ptr = hm_get(chunks, current_idx);
        if (!chunk_ptr || !*chunk_ptr || !(*chunk_ptr)->ready) {
            continue;
        }
        chunk_t *c = *chunk_ptr;

        uint32_t tile_count = c->tiles.renderer.count / 6;
        for (uint32_t t = 0; t < tile_count; t++) {
            if (!c->tiles.renderer.collidable[t]) {
                continue;
            }

            vec2f tile_pos = VEC2F(c->tiles.renderer.vertices[(size_t)t * 6].x, c->tiles.renderer.vertices[(size_t)t * 6].y);

            if (player->attribs.pos.x + player->attribs.size.x <= tile_pos.x || player->attribs.pos.x >= tile_pos.x + BLOCK_SIZE) {
                continue;
            }

            rect_collider player_collider = {
                .pos = player->attribs.pos, .size = player->attribs.size
            };
            rect_collider tile_collider = {
                .pos = tile_pos, .size = VEC2F(BLOCK_SIZE, BLOCK_SIZE)
            };

            if (check_collision_rects(player_collider, tile_collider)) {
                if (player->attribs.vel.y > 0) {
                    player->attribs.pos.y = tile_pos.y - player->attribs.size.y - 0.01f;
                    player->attribs.ground = true;
                } else if (player->attribs.vel.y < 0) {
                    player->attribs.pos.y = tile_pos.y + BLOCK_SIZE + 0.01f; 
                }
                player->attribs.vel.y = 0;
            }
        }
    }
}

#pragma endregion

void player_update(player_t *player, chunk_entry *chunks, block_data_t *block_data, item_drop_t **drops, 
                   uint32_t left_most_chunk, uint32_t right_most_chunk) {
    assert(player);
    assert(chunks);
    assert(block_data);
    assert(drops);

    player_handle_controls(player, chunks, block_data, drops, left_most_chunk, right_most_chunk);
    player_move_and_collide(player, chunks, left_most_chunk, right_most_chunk);
}

uint32_t player_set_spawn_point(player_t *player, fnl_state *terrain) {
    uint32_t spawn_chunk_x = (uint32_t)(MAP_SIZE * 0.5f);
    float noise = (fnlGetNoise2D(terrain, (float)((uint32_t)(spawn_chunk_x * CHUNK_SIZE)), 0) + 1.0f) * 0.5f;
    uint32_t height = MIN_TERRAIN_HEIGHT + (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));
    player->attribs.pos.x = (float)spawn_chunk_x * CHUNK_SIZE * BLOCK_SIZE;
    player->attribs.pos.y = (float)(MAX_CHUNK_HEIGHT - height - 1) * BLOCK_SIZE;
    return spawn_chunk_x;
}

#pragma region UI

void player_draw_inventory(player_t *player, texture *inventory, texture *item_textures, texture *hotbar_arrow, font *f) {
    if (player->inventory.enabled) {
        begin_draw(SHAPE_2D_UNLIT, false);
        draw_rect(VEC2F(0, 0), get_screen_size(), RGBA(0, 0, 0, 125), 0);
        begin_draw(TEXTURE_2D_UNLIT, false);
        vec2f size = VEC2F(inventory->size.x * 4, inventory->size.y * 4);
        vec2f pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f), (get_screen_height() * 0.5f) - (size.y * 0.5f));
        draw_texture2D(inventory, pos, size, WHITE, NO_ROTATION);

        vec2f first_hotbar_slot = VEC2F(pos.x + (4 * 4), pos.y + size.y - (24 * 4));
        for (uint32_t i = 0; i < 9; i++) {
            if (player->inventory.hotbar[i].count == 0 ||
                player->inventory.hotbar[i].item == ITEM_NONE) {
                continue;
            }
            vec2f arrow_size = VEC2F(hotbar_arrow->size.x * 4, hotbar_arrow->size.y * 4);
            vec2f item_size = VEC2F(item_textures[player->inventory.hotbar[i].item].size.x * 4, 
                                    item_textures[player->inventory.hotbar[i].item].size.y * 4);
            vec2f slot_pos = VEC2F(first_hotbar_slot.x + (arrow_size.x * 0.5f) - (item_size.x * 0.5f) + ((18 * 4) * i), first_hotbar_slot.y);
            draw_texture2D(&item_textures[player->inventory.hotbar[i].item], slot_pos, item_size, WHITE, NO_ROTATION);
        }

        begin_draw(TEXT, false);
        for (uint32_t i = 0; i < 9; i++) {
            vec2f arrow_size = VEC2F(hotbar_arrow->size.x * 4, hotbar_arrow->size.y * 4);
            vec2f item_size = VEC2F(item_textures[player->inventory.hotbar[i].item].size.x * 4, 
                                    item_textures[player->inventory.hotbar[i].item].size.y * 4);
            float offset_y = 20.0f;
            vec2f slot_pos = VEC2F(first_hotbar_slot.x + (arrow_size.x * 0.5f) - (item_size.x * 0.5f) + ((18 * 4) * i), first_hotbar_slot.y);
            if (player->inventory.hotbar[i].count <= 1 || player->inventory.hotbar[i].item == ITEM_NONE) {
                continue;
            }
    
            vec2f text_pos = VEC2F(slot_pos.x + arrow_size.x - (12 * 4), slot_pos.y + arrow_size.y - (12 * 4));
            draw_text_shadow(f, text_pos, 0.7f, WHITE, VEC2F(4, 4), BLACK, "%d", player->inventory.hotbar[i].count);
        }
    }
}

void player_draw_ui(player_t *player, texture *hotbar, texture *hotbar_arrow, texture *item_textures, font *f) {
    if (!player->inventory.enabled) {
        begin_draw(TEXTURE_2D_UNLIT, false);

        vec2f size = VEC2F(hotbar->size.x * 4, hotbar->size.y * 4);
        float offset_y = 20.0f;
        vec2f hotbar_pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f), get_screen_height() - size.y - offset_y);
        draw_texture2D(hotbar, hotbar_pos, size, WHITE, NO_ROTATION);

        vec2f arrow_size = VEC2F(hotbar_arrow->size.x * 4, hotbar_arrow->size.y * 4);
        vec2f arrow_pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f) + ((arrow_size.x - (4 * 4)) * player->inventory.hotbar_selected) - 4,
                                get_screen_height() - size.y - offset_y - 4);
        draw_texture2D(hotbar_arrow, arrow_pos, arrow_size, WHITE, NO_ROTATION);

        for (uint32_t i = 0; i < 9; i++) {
            vec2f slot_pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f) + ((arrow_size.x - (4 * 4)) * i) - 4, 
                                   get_screen_height() - size.y - offset_y - 4);
            if (player->inventory.hotbar[i].count == 0 || player->inventory.hotbar[i].item == ITEM_NONE) {
                continue;
            }
            vec2f item_size = VEC2F(item_textures[player->inventory.hotbar[i].item].size.x * 4, 
                                    item_textures[player->inventory.hotbar[i].item].size.y * 4);
            draw_texture2D(&item_textures[player->inventory.hotbar[i].item], 
                           VEC2F(slot_pos.x + (arrow_size.x * 0.5f) - (item_size.x * 0.5f), slot_pos.y + (arrow_size.y * 0.5f) - (item_size.y * 0.5f)),
                           item_size, WHITE, NO_ROTATION);
        }

        tilemap_begin_editing(&player->stats.icons_bg);
        float icon_offset = 10.0f;
        for (uint32_t i = 0; i < 10; i++) {
            vec2f icon_pos = VEC2F(hotbar_pos.x + ((ICON_PIXEL_SIZE) * 4 * i), hotbar_pos.y - (ICON_PIXEL_SIZE * 4) - icon_offset);
            tilemap_add_tile(&player->stats.icons_bg, icon_pos, VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HEART_BG);
        }

        vec2f last_heart_pos = VEC2F(hotbar_pos.x + ((ICON_PIXEL_SIZE) * 4 * 9), hotbar_pos.y - (ICON_PIXEL_SIZE * 4) - icon_offset);

        float hunger_bar_offset = 10.0f * 4;
        for (uint32_t i = 0; i < 10; i++) {
            vec2f icon_pos = VEC2F(last_heart_pos.x + (ICON_PIXEL_SIZE * 4) + ((ICON_PIXEL_SIZE - 1) * 4 * i) + hunger_bar_offset, last_heart_pos.y);
            tilemap_add_tile(&player->stats.icons_bg, icon_pos, VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HUNGER_BG);
        }

        tilemap_begin_editing(&player->stats.icons);
        for (uint32_t i = 0; i < (uint32_t)(player->stats.health * 0.5f); i++) {
            vec2f icon_pos = VEC2F(hotbar_pos.x + ((ICON_PIXEL_SIZE) * 4 * i), hotbar_pos.y - (ICON_PIXEL_SIZE * 4) - icon_offset);
            tilemap_add_tile(&player->stats.icons, icon_pos, VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HEART);
        }
        int health_left = (int)player->stats.health % 2;
        if (health_left == 1) {
            vec2f icon_pos = VEC2F(hotbar_pos.x + ((ICON_PIXEL_SIZE) * 4 * (uint32_t)(player->stats.health * 0.5f)), 
                                   hotbar_pos.y - (ICON_PIXEL_SIZE * 4) - icon_offset);
            tilemap_add_tile(&player->stats.icons, icon_pos, VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HEART_HALF);
        }

        for (uint32_t i = 0; i < (uint32_t)(player->stats.hunger * 0.5f); i++) {
            vec2f icon_pos = VEC2F(last_heart_pos.x + (ICON_PIXEL_SIZE * 4) + ((ICON_PIXEL_SIZE - 1) * 4 * i) + hunger_bar_offset, last_heart_pos.y);
            tilemap_add_tile(&player->stats.icons, icon_pos, VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HUNGER);
        }
        int hunger_left = (int)player->stats.health % 2;
        if (hunger_left == 1) {
            vec2f icon_pos = VEC2F(last_heart_pos.x + (ICON_PIXEL_SIZE * 4) + ((ICON_PIXEL_SIZE - 1) * 4 * (uint32_t)(player->stats.hunger * 0.5f)) +
                                   hunger_bar_offset, last_heart_pos.y);
            tilemap_add_tile(&player->stats.icons, icon_pos, VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HUNGER_HALF);
        }

        tilemap_draw(&player->stats.icons_bg, WHITE);
        tilemap_draw(&player->stats.icons, WHITE);

        begin_draw(TEXT, false);

        for (uint32_t i = 0; i < 9; i++) {
            vec2f slot_pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f) + ((arrow_size.x - (4 * 4)) * i) - 4,
                                   get_screen_height() - size.y - offset_y - 4);
            if (player->inventory.hotbar[i].count <= 1 || player->inventory.hotbar[i].item == ITEM_NONE) {
                continue;
            }

            vec2f text_pos = VEC2F(slot_pos.x + arrow_size.x - (9 * 4), slot_pos.y + arrow_size.y - (9 * 4));
            draw_text_shadow(f, text_pos, 0.7f, WHITE, VEC2F(4, 4), BLACK, "%d", player->inventory.hotbar[i].count);
        }
    }
}

#pragma endregion
