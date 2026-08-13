#include "player.h"
#include "blocks.h"
#include "chunk.h"
#include "items.h"
#include "textures.h"

#include <cpstd/hashmap.h>
#include <cpstd/mathplus.h>
#include <cpstd/vector.h>
#include <cpl/cpl.h>

physical_attribs_t attribs = {
    .pos = VEC2F(0, 0),
    .size = VEC2F(0.5f * BLOCK_SIZE, 1.75f * BLOCK_SIZE),
    .vel = VEC2F(0, 0),
    .ground = true,
    .jmp_force = 450.0f,
    .gravity = 900.0f,
    .move_speed = PLAYER_BASE_SPEED,
    .max_fall_speed = 1100.0f
};
mining_t mining = {
    .block = VEC2F(-1, -1), 
    .block_dt = 0.0f, 
    .timer = 0.0f
};
stats_t stats = {
    .health = 20, 
    .hunger = 20
};
inventory_t inventory = {
    .hotbar_selected = 0, 
    .enabled = false
};

mining_t *player_get_mining_properties() {
    return &mining;
}

stats_t *player_get_stats_properties() {
    return &stats;
}

inventory_t *player_get_inventory_properties() {
    return &inventory;
}

physical_attribs_t *player_get_attribs_properties() {
    return &attribs;
}

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

void player_draw() {
    draw_rect(attribs.pos, attribs.size, RED, NO_ROTATION);
}

// If tilemap texture gets bigger, it is more vulnerable to cumbersome precision
// Additionally because of insetting of tilemap uvs in cpl we need a larger uv epsilon
int player_get_block_type_id(vec2f uv) {
    for (int i = 0; i < BLOCK_TYPE_T_SIZE; i++) {
        if (math_abs(uv.x - blocks_get_block_data(i)->uv.x) < UV_EPSILON &&
            math_abs(uv.y - blocks_get_block_data(i)->uv.y) < UV_EPSILON) {
            return i;
        }
    }
    return -1;
}

#pragma region player_update() Helper

#pragma region player_handle_controls() Helper

void player_handle_movement() {
    if (!inventory.enabled) {
        if (is_key_down(KEY_LETTER_A)) {
            attribs.vel.x = -attribs.move_speed;
        } else if (is_key_down(KEY_LETTER_D)) {
            attribs.vel.x = attribs.move_speed;
        } else {
            attribs.vel.x = 0;
        }
        if (is_key_down(KEY_SPACE) && attribs.ground) {
            attribs.vel.y = -attribs.jmp_force;
            attribs.ground = false;
        }
    } else {
        attribs.vel.x = 0;
    }
    attribs.vel.y += attribs.gravity * get_dt();
    if (attribs.vel.y > attribs.max_fall_speed) {
        attribs.vel.y = attribs.max_fall_speed;
    }
    if (!inventory.enabled) {
        if (is_key_down(KEY_LEFT_SHIFT)) {
            attribs.move_speed = PLAYER_BASE_SPEED * 2;
        } else {
            attribs.move_speed = PLAYER_BASE_SPEED;
        }
    }
    attribs.move_speed = math_clamp(attribs.move_speed, 10, 1000000);
}

vec2f player_raycast_hit_tile(vec2f origin, vec2f d, float max_dist) {
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
            vec2f uv = tilemap_get_tile_uv(&(*hm_get(chunk_get_chunkmap(), chunk_idx))->tiles_passable, tile);
            vec2f water_uv = blocks_get_block_data(BLOCK_WATER)->uv;
            bool is_water = math_abs(uv.x - water_uv.x) < UV_EPSILON && math_abs(uv.y - water_uv.y) < UV_EPSILON;
            if (tilemap_tile_exists(&(*hm_get(chunk_get_chunkmap(), chunk_idx))->tiles, tile) ||
                (tilemap_tile_exists(&(*hm_get(chunk_get_chunkmap(), chunk_idx))->tiles_passable, tile) && !is_water)) {
                return tile;
            }
        }
    }
    return VEC2F(-1, -1);
}

bool player_neighbor_blocks_exist(vec2f mouse_pos_tilemap, uint32_t idx) {
    vec2f neighbor_blocks[4] = {
        {mouse_pos_tilemap.x + BLOCK_SIZE, mouse_pos_tilemap.y},
        {mouse_pos_tilemap.x - BLOCK_SIZE, mouse_pos_tilemap.y},
        {mouse_pos_tilemap.x, mouse_pos_tilemap.y + BLOCK_SIZE},
        {mouse_pos_tilemap.x, mouse_pos_tilemap.y - BLOCK_SIZE}
    };
    for (int c = -1; c < 2; c++) {
        if (c == -1 && idx == chunk_get_leftmost_idx()) {
            continue;
        }
        if (c == 1 && idx == chunk_get_rightmost_idx()) {
            continue;
        }
        for (uint32_t i = 0; i < 4; i++) {
            if (tilemap_tile_exists(&(*hm_get(chunk_get_chunkmap(), idx + c))->tiles, neighbor_blocks[i])) {
                return true;
                break;
            }
        }
    }
    return false;
}

#pragma region player_handle_block_placing() Helper

bool player_can_place_block(rect_collider_t player_collider, rect_collider_t tile_collider, vec2f mouse_pos_tilemap, uint32_t idx, int block_id) {
    return !check_collision_rects(player_collider, tile_collider) && 
           !tilemap_tile_exists(&(*hm_get(chunk_get_chunkmap(), idx))->tiles, mouse_pos_tilemap) &&
           player_neighbor_blocks_exist(mouse_pos_tilemap, idx) && block_id != -1 && inventory.hotbar[inventory.hotbar_selected].count > 0;
}

#pragma endregion

void player_handle_block_placing(vec2f mouse_pos, vec2f mouse_pos_tilemap) {
    if (is_mouse_pressed(MOUSE_BUTTON_RIGHT)) {
        if (vec2f_dist(attribs.pos, mouse_pos) > MINE_AND_PLACE_RANGE * BLOCK_SIZE) {
            return;
        }
        if (mouse_pos.x < (float)chunk_get_leftmost_idx() * CHUNK_SIZE * BLOCK_SIZE || 
            mouse_pos.x > (float)(chunk_get_rightmost_idx() + 1) * CHUNK_SIZE * BLOCK_SIZE) {
            return;
        }
        uint32_t idx = (uint32_t)mouse_pos.x / (CHUNK_SIZE * BLOCK_SIZE);
        rect_collider_t player_collider = {
            .pos = attribs.pos,
            .size = attribs.size
        };
        rect_collider_t tile_collider = {
            .pos = mouse_pos_tilemap,
            .size = VEC2F(BLOCK_SIZE, BLOCK_SIZE)
        };

        int block_id = player_item_id_to_block_id(inventory.hotbar[inventory.hotbar_selected].item);
        if (player_can_place_block(player_collider, tile_collider, mouse_pos_tilemap, idx, block_id)) {
            vec2f uv = VEC2F(0, 0);
            if (tilemap_tile_exists(&(*hm_get(chunk_get_chunkmap(), idx))->tiles_passable, mouse_pos_tilemap)) {
                uv = tilemap_get_tile_uv(&(*hm_get(chunk_get_chunkmap(), idx))->tiles_passable, mouse_pos_tilemap);
                vec2f water_uv = blocks_get_block_data(BLOCK_WATER)->uv;
                bool is_water = math_abs(uv.x - water_uv.x) < UV_EPSILON && math_abs(uv.y - water_uv.y) < UV_EPSILON;
                if (is_water && !blocks_get_block_data(block_id)->passable) {
                    tilemap_delete_tile(&(*hm_get(chunk_get_chunkmap(), idx))->tiles_passable, mouse_pos_tilemap);
                    tilemap_add_tile(&(*hm_get(chunk_get_chunkmap(), idx))->tiles, mouse_pos_tilemap, VEC2F(BLOCK_SIZE, BLOCK_SIZE), 
                                     blocks_get_block_data(block_id)->uv);
                    tilemap_check_collidable_tiles(&(*hm_get(chunk_get_chunkmap(), idx))->tiles, VEC2F(BLOCK_SIZE, BLOCK_SIZE));

                    inventory.hotbar[inventory.hotbar_selected].count--;
                    if (inventory.hotbar[inventory.hotbar_selected].count == 0) {
                        inventory.hotbar[inventory.hotbar_selected].item = ITEM_NONE;
                    }
                }
            } else {
                if (blocks_get_block_data(block_id)->passable) {
                    tilemap_add_tile(&(*hm_get(chunk_get_chunkmap(), idx))->tiles_passable, mouse_pos_tilemap, VEC2F(BLOCK_SIZE, BLOCK_SIZE), 
                                     blocks_get_block_data(block_id)->uv);
                } else {
                    tilemap_add_tile(&(*hm_get(chunk_get_chunkmap(), idx))->tiles, mouse_pos_tilemap, VEC2F(BLOCK_SIZE, BLOCK_SIZE), 
                                     blocks_get_block_data(block_id)->uv);
                    tilemap_check_collidable_tiles(&(*hm_get(chunk_get_chunkmap(), idx))->tiles, VEC2F(BLOCK_SIZE, BLOCK_SIZE));
                }

                inventory.hotbar[inventory.hotbar_selected].count--;
                if (inventory.hotbar[inventory.hotbar_selected].count == 0) {
                    inventory.hotbar[inventory.hotbar_selected].item = ITEM_NONE;
                }
            }
        }
    }
}

void player_handle_block_breaking(item_drop_t **drops, vec2f mouse_pos, vec2f mouse_pos_tilemap) {
    if (is_mouse_released(MOUSE_BUTTON_LEFT)) {
        mining.block = VEC2F(-1, -1);
        mining.block_dt = 0.0f;
    }
    if (is_mouse_down(MOUSE_BUTTON_LEFT)) {
        if (vec2f_dist(attribs.pos, mouse_pos) > MINE_AND_PLACE_RANGE * BLOCK_SIZE) {
            mining.block = VEC2F(-1, -1);
            mining.block_dt = 0.0f;
            return;
        }
        if (mouse_pos.x < (float)chunk_get_leftmost_idx() * CHUNK_SIZE * BLOCK_SIZE || 
            mouse_pos.x > (float)(chunk_get_rightmost_idx() + 1) * CHUNK_SIZE * BLOCK_SIZE) {
            return;
        }
        uint32_t idx = (uint32_t)mouse_pos.x / (CHUNK_SIZE * BLOCK_SIZE);
        vec2f ray_dir = VEC2F(mouse_pos.x - attribs.pos.x, mouse_pos.y - attribs.pos.y);
        if (!vec2f_cmp(player_raycast_hit_tile(attribs.pos, ray_dir, MINE_AND_PLACE_RANGE * BLOCK_SIZE), mouse_pos_tilemap)) {
            mining.block = VEC2F(-1, -1);
            mining.block_dt = 0.0f;
            return;
        }
        vec2f uv = VEC2F(0, 0);
        if (tilemap_tile_exists(&(*hm_get(chunk_get_chunkmap(), idx))->tiles_passable, mouse_pos_tilemap)) {
            uv = tilemap_get_tile_uv( &(*hm_get(chunk_get_chunkmap(), idx))->tiles_passable, mouse_pos_tilemap);
        } else if (tilemap_tile_exists(&(*hm_get(chunk_get_chunkmap(), idx))->tiles, mouse_pos_tilemap)) {
            uv = tilemap_get_tile_uv(&(*hm_get(chunk_get_chunkmap(), idx))->tiles, mouse_pos_tilemap);
        }
        int block_id = player_get_block_type_id(uv);

        if (!vec2f_cmp(mouse_pos_tilemap, mining.block)) {
            mining.block = mouse_pos_tilemap;
            mining.timer = get_time();

            if (block_id == -1 || blocks_get_block_data(block_id)->unbreakable) {
                mining.block = VEC2F(-1, -1);
                mining.block_dt = 0.0f;
            } else {
                mining.block_dt = blocks_get_block_data(block_id)->base_mining_dt;
            }
        } else if (mining.timer + mining.block_dt <= get_time() && !vec2f_cmp(mining.block, VEC2F(-1, -1))) {
            int item_id = player_block_id_to_item_id(block_id);
            if (item_id != -1 && item_id != ITEM_OAK_LEAVES &&
                item_id != ITEM_GRASS) {
                vec_push(*drops, (item_drop_t){});
                if (block_id == BLOCK_GRASS_BLOCK) {
                    items_drop_item(vec_back(*drops), ITEM_DIRT, mouse_pos);
                } else if (block_id == BLOCK_STONE) {
                    items_drop_item(vec_back(*drops), ITEM_COBBLESTONE, mouse_pos);
                } else if (block_id == BLOCK_COAL_ORE) {
                    items_drop_item(vec_back(*drops), ITEM_COAL, mouse_pos);
                } else if (block_id == BLOCK_DIAMOND_ORE) {
                    items_drop_item(vec_back(*drops), ITEM_DIAMOND, mouse_pos);
                } else {
                    items_drop_item(vec_back(*drops), item_id, mouse_pos);
                }
            }

            if (tilemap_tile_exists(&(*hm_get(chunk_get_chunkmap(), idx))->tiles_passable, mouse_pos_tilemap)) {
                tilemap_delete_tile(&(*hm_get(chunk_get_chunkmap(), idx))->tiles_passable, mouse_pos_tilemap);
            } else if (tilemap_tile_exists(&(*hm_get(chunk_get_chunkmap(), idx))->tiles, mouse_pos_tilemap)) {
                tilemap_delete_tile(&(*hm_get(chunk_get_chunkmap(), idx))->tiles, mouse_pos_tilemap);
                tilemap_check_collidable_tiles(&(*hm_get(chunk_get_chunkmap(), idx))->tiles, VEC2F(BLOCK_SIZE, BLOCK_SIZE));
            }

            mining.block = VEC2F(-1, -1);
            mining.block_dt = 0.0f;
            mining.timer = 0.0f;
        }
    }
}

void player_handle_item_drops(item_drop_t **drops, vec2f mouse_pos, vec2f mouse_pos_tilemap) {
    if (is_key_pressed(KEY_LETTER_Q)) {
        item_type_t type = inventory.hotbar[inventory.hotbar_selected].item;
        uint32_t idx = (uint32_t)mouse_pos.x / (CHUNK_SIZE * BLOCK_SIZE);
        if (idx < chunk_get_leftmost_idx() || idx > chunk_get_rightmost_idx()) {
            return;
        }
        if (type != ITEM_NONE && inventory.hotbar[inventory.hotbar_selected].count > 0 && 
            !tilemap_tile_exists(&(*hm_get(chunk_get_chunkmap(), idx))->tiles, mouse_pos_tilemap)) {
            vec_push(*drops, (item_drop_t){});
            items_drop_item(vec_back(*drops), type, mouse_pos);

            inventory.hotbar[inventory.hotbar_selected].count--;
            if (inventory.hotbar[inventory.hotbar_selected].count == 0) {
                inventory.hotbar[inventory.hotbar_selected].item = ITEM_NONE;
            }
        }
    }

    {
        uint32_t w = 0;
        for (uint32_t i = 0; i < vec_size(*drops); i++) {
            item_drop_t drop = (*drops)[i];
            rect_collider_t drop_collider = {
                .pos = drop.pos, 
                .size = drop.size
            };
            rect_collider_t player_collider = {
                .pos = attribs.pos,
                .size = attribs.size
            };

            if (!check_collision_rects(player_collider, drop_collider)) {
                (*drops)[w++] = (*drops)[i];
            } else {
                bool item_picked_up = false;
                // Hotbar pickup add to stack
                for (uint32_t j = 0; j < 9; j++) {
                    item_type_t type = drop.type;
                    if (inventory.hotbar[j].item == type && inventory.hotbar[j].count < MAX_STACK_SIZE) {
                        inventory.hotbar[j].count++;
                        item_picked_up = true;
                        break;
                    }
                }
                // Inventory pickup add to stack
                if (!item_picked_up) {
                    for (int j = MAX_INVENTORY_SLOTS; j >= 0; j--) {
                        item_type_t type = drop.type;
                        if (inventory.slots[j].item == type && inventory.slots[j].count < MAX_STACK_SIZE) {
                            inventory.slots[j].count++;
                            item_picked_up = true;
                            break;
                        } 
                    }
                }
                // Hotbar pickup as new stack
                if (!item_picked_up) {
                    for (uint32_t j = 0; j < 9; j++) {
                        item_type_t type = drop.type;
                        if (inventory.hotbar[j].item == ITEM_NONE && inventory.hotbar[j].count == 0) {
                            inventory.hotbar[j].item = type;
                            inventory.hotbar[j].count++;
                            item_picked_up = true;
                            break;
                        }
                    }
                }
                // Inventory pickup as new stack
                if (!item_picked_up) {
                    for (int j = MAX_INVENTORY_SLOTS; j >= 0; j--) {
                        item_type_t type = drop.type;
                        if (inventory.slots[j].item == ITEM_NONE && inventory.slots[j].count == 0) {
                            inventory.slots[j].item = type;
                            inventory.slots[j].count++;
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
        items_update_drop(drop);
    }
    vec_erase_if(drop, *drops, drop->timer + ITEM_DROP_LIFETIME <= get_time());
}

void player_handle_hotbar() {
    if (get_mouse_scroll_offset().y > 0) {
        inventory.hotbar_selected = (inventory.hotbar_selected + 1) % MAX_HOTBAR_SLOTS;
    } else if (get_mouse_scroll_offset().y < 0) {
        if (inventory.hotbar_selected == 0) {
            inventory.hotbar_selected = MAX_HOTBAR_SLOTS - 1;
        } else {
            inventory.hotbar_selected--;
        }
    }
    if (is_key_down(KEY_DIGIT_1)) {
        inventory.hotbar_selected = 0;
    }
    if (is_key_down(KEY_DIGIT_2)) {
        inventory.hotbar_selected = 1;
    }
    if (is_key_down(KEY_DIGIT_3)) {
        inventory.hotbar_selected = 2;
    }
    if (is_key_down(KEY_DIGIT_4)) {
        inventory.hotbar_selected = 3;
    }
    if (is_key_down(KEY_DIGIT_5)) {
        inventory.hotbar_selected = 4;
    }
    if (is_key_down(KEY_DIGIT_6)) {
        inventory.hotbar_selected = 5;
    }
    if (is_key_down(KEY_DIGIT_7)) {
        inventory.hotbar_selected = 6;
    }
    if (is_key_down(KEY_DIGIT_8)) {
        inventory.hotbar_selected = 7;
    }
    if (is_key_down(KEY_DIGIT_9)) {
        inventory.hotbar_selected = 8;
    }
}

#pragma region player_handle_inventory() Helper

int player_get_slot() {
    vec2f size = VEC2F(textures_get_ui_texture(TEXTURE_INVENTORY)->size.x * 4, textures_get_ui_texture(TEXTURE_INVENTORY)->size.y * 4);
    vec2f pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f), (get_screen_height() * 0.5f) - (size.y * 0.5f));
    vec2f arrow_size = VEC2F(textures_get_ui_texture(TEXTURE_HOTBAR_ARROW)->size.x * 4, 
                             textures_get_ui_texture(TEXTURE_HOTBAR_ARROW)->size.y * 4);
    vec2f item_size = VEC2F(textures_get_item_texture(ITEM_DIRT)->size.x * 4, 
                            textures_get_item_texture(ITEM_DIRT)->size.y * 4);
    vec2f first_inventory_slot = VEC2F(pos.x + (4 * 4), pos.y + (84 * 4));
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 9; x++) {
            vec2f slot_pos = VEC2F(first_inventory_slot.x + (arrow_size.x * 0.5f) - (item_size.x * 0.5f) + ((18 * 4) * x),
                                    first_inventory_slot.y + ((18 * 4) * y));
            rect_collider_t slot_collider = {
                .pos = slot_pos,
                .size = item_size
            };
            if (check_collision_vec2f_rect(get_mouse_pos(), slot_collider)) {
                return (y * 9) + x;
            }
        }
    }

    vec2f first_hotbar_slot = VEC2F(pos.x + (4 * 4), pos.y + size.y - (24 * 4));
    for (int i = 0; i < 9; i++) {
        vec2f slot_pos = VEC2F(first_hotbar_slot.x + (arrow_size.x * 0.5f) - (item_size.x * 0.5f) + ((18 * 4) * i), first_hotbar_slot.y);
        rect_collider_t slot_collider = {
            .pos = slot_pos,
            .size = item_size
        };
        if (check_collision_vec2f_rect(get_mouse_pos(), slot_collider)) {
            return 27 + i;
        }
    }
    return -1;
}

#pragma endregion

int slot_selected = -1;
slot_t item_dragged = {
    .item = ITEM_NONE,
    .count = 0
};

void player_handle_inventory() {
    int slot_idx = player_get_slot();
    // Pickup half of items in the slot
    if (is_mouse_pressed(MOUSE_BUTTON_RIGHT) && slot_idx != -1 && slot_selected == -1) {
        if (slot_idx >= 27) {
            uint32_t half_count = inventory.hotbar[slot_idx - 27].count / 2;
            if (half_count > 0) {
                inventory.hotbar[slot_idx - 27].count -= half_count;
                item_dragged.item = inventory.hotbar[slot_idx - 27].item;
                item_dragged.count = half_count;
                slot_selected = slot_idx;
                // Huh never thought I will need it
                goto _skip_put_single_item;
            }
        } else {
            uint32_t half_count = inventory.slots[slot_idx].count / 2;
            if (half_count > 0) {
                inventory.slots[slot_idx].count -= half_count;
                item_dragged.item = inventory.slots[slot_idx].item;
                item_dragged.count = half_count;
                slot_selected = slot_idx;
                // Huh never thought I will need it
                goto _skip_put_single_item;
            }
        }
    }
    // Put single item from dragged items into a slot
    if (is_mouse_pressed(MOUSE_BUTTON_RIGHT) && slot_idx != -1 && slot_selected != -1 &&
        item_dragged.item != ITEM_NONE && item_dragged.count > 0) {
        if (slot_idx >= 27) {
            if (inventory.hotbar[slot_idx - 27].count < 64) {
                if (inventory.hotbar[slot_idx - 27].item == item_dragged.item) {
                    inventory.hotbar[slot_idx - 27].count++;
                    item_dragged.count--;
                } else if (inventory.hotbar[slot_idx - 27].item == ITEM_NONE && inventory.hotbar[slot_idx - 27].count == 0) {
                    inventory.hotbar[slot_idx - 27].item = item_dragged.item;
                    inventory.hotbar[slot_idx - 27].count++;
                    item_dragged.count--; 
                }
            }
        } else {
            if (inventory.slots[slot_idx].count < 64) {
                if (inventory.slots[slot_idx].item == item_dragged.item) {
                    inventory.slots[slot_idx].count++;
                    item_dragged.count--;
                } else if (inventory.slots[slot_idx].item == ITEM_NONE && inventory.slots[slot_idx].count == 0) {
                    inventory.slots[slot_idx].item = item_dragged.item;
                    inventory.slots[slot_idx].count++;
                    item_dragged.count--; 
                }
            }
        }

        if (item_dragged.count == 0) {
            item_dragged.item = ITEM_NONE;
            slot_selected = -1;
        }
    }
_skip_put_single_item:
    // Pickup whole slot
    if (is_mouse_pressed(MOUSE_BUTTON_LEFT) && slot_idx != -1) {
        if (slot_selected != -1) {
            if (slot_idx >= 27) {
                if (inventory.hotbar[slot_idx - 27].item == ITEM_NONE && inventory.hotbar[slot_idx - 27].count == 0) {
                    inventory.hotbar[slot_idx - 27].item = item_dragged.item;
                    inventory.hotbar[slot_idx - 27].count = item_dragged.count;
                    item_dragged.item = ITEM_NONE;
                    item_dragged.count = 0;
                    slot_selected = -1;
                } else {
                    if (inventory.hotbar[slot_idx - 27].item == item_dragged.item) {
                        if (inventory.hotbar[slot_idx - 27].count + item_dragged.count > 64) {
                            uint32_t items_left = inventory.hotbar[slot_idx - 27].count + (int)item_dragged.count - 64;
                            inventory.hotbar[slot_idx - 27].count = 64;
                            item_dragged.count = items_left;
                        } else {
                            inventory.hotbar[slot_idx - 27].count += item_dragged.count;
                            item_dragged.item = ITEM_NONE;
                            item_dragged.count = 0;
                            slot_selected = -1;
                        }
                    } else {
                        slot_t target = {
                            .item = inventory.hotbar[slot_idx - 27].item,
                            .count = inventory.hotbar[slot_idx - 27].count
                        };
                        inventory.hotbar[slot_idx - 27].item = item_dragged.item;
                        inventory.hotbar[slot_idx - 27].count = item_dragged.count;
                        item_dragged = target;
                        slot_selected = slot_idx;
                    }
                }
            } else {
                if (inventory.slots[slot_idx].item == ITEM_NONE && inventory.slots[slot_idx].count == 0) {
                    inventory.slots[slot_idx].item = item_dragged.item;
                    inventory.slots[slot_idx].count = item_dragged.count;
                    item_dragged.item = ITEM_NONE;
                    item_dragged.count = 0;
                    slot_selected = -1;
                } else {
                    if (inventory.slots[slot_idx].item == item_dragged.item) {
                        if (inventory.slots[slot_idx].count + item_dragged.count > 64) {
                            uint32_t items_left = inventory.slots[slot_idx].count + (int)item_dragged.count - 64;
                            inventory.slots[slot_idx].count = 64;
                            item_dragged.count = items_left;
                        } else {
                            inventory.slots[slot_idx].count += item_dragged.count;
                            item_dragged.item = ITEM_NONE;
                            item_dragged.count = 0;
                            slot_selected = -1;
                        }
                    } else {
                        slot_t target = {
                            .item = inventory.slots[slot_idx].item,
                            .count = inventory.slots[slot_idx].count
                        };
                        inventory.slots[slot_idx].item = item_dragged.item;
                        inventory.slots[slot_idx].count = item_dragged.count;
                        item_dragged = target;
                        slot_selected = slot_idx;
                    }
                }
            } 
        } else {
            slot_selected = slot_idx;
            if (slot_idx >= 27) {
                item_dragged.item = inventory.hotbar[slot_idx - 27].item;
                item_dragged.count = inventory.hotbar[slot_idx - 27].count;
                inventory.hotbar[slot_idx - 27].item = ITEM_NONE;
                inventory.hotbar[slot_idx - 27].count = 0;
            } else {
                item_dragged.item = inventory.slots[slot_idx].item;
                item_dragged.count = inventory.slots[slot_idx].count;
                inventory.slots[slot_idx].item = ITEM_NONE;
                inventory.slots[slot_idx].count = 0;
            }
        }
    }
}

#pragma endregion

void player_handle_controls(item_drop_t **drops) {
    get_cam_2D()->pos = VEC2F(attribs.pos.x - (get_screen_width() * (1 / get_cam_2D()->zoom) * 0.5f),
                              attribs.pos.y - (get_screen_height() * (1 / get_cam_2D()->zoom) * 0.5f));
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
        inventory.enabled = !inventory.enabled;
    }

    player_handle_movement();
    if (!inventory.enabled) {
        player_handle_hotbar();
        vec2f mouse_pos = get_screen_to_world_2D(get_mouse_pos());
        vec2f mouse_pos_tilemap = VEC2F((int)mouse_pos.x - ((int)mouse_pos.x % BLOCK_SIZE), (int)mouse_pos.y - ((int)mouse_pos.y % BLOCK_SIZE));

        player_handle_block_placing(mouse_pos, mouse_pos_tilemap);
        player_handle_block_breaking(drops, mouse_pos, mouse_pos_tilemap);
        player_handle_item_drops(drops, mouse_pos, mouse_pos_tilemap);
    } else {
        player_handle_inventory();
    }
}

// TODO make move and collide better and not glitchy to get under the ground

void player_move_and_collide() {
    float dt = get_dt();
    
    attribs.pos.x += attribs.vel.x * dt;
    int idx = (int)attribs.pos.x / (CHUNK_SIZE * BLOCK_SIZE);

    if (idx < (int)chunk_get_leftmost_idx() || idx > (int)chunk_get_rightmost_idx()) {
        return;
    }

    for (int i = -1; i < 2; i++) {
        int current_idx = idx + i;
        if (current_idx < (int)chunk_get_leftmost_idx() || current_idx > (int)chunk_get_rightmost_idx()) {
            continue;
        }

        chunk_t **chunk_ptr = hm_get(chunk_get_chunkmap(), current_idx);
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

            if (attribs.pos.y + attribs.size.y <= tile_pos.y || attribs.pos.y >= tile_pos.y + BLOCK_SIZE) {
                continue;
            }

            rect_collider_t player_collider = {
                .pos = attribs.pos, 
                .size = attribs.size
            };
            rect_collider_t tile_collider = {
                .pos = tile_pos, 
                .size = VEC2F(BLOCK_SIZE, BLOCK_SIZE)
            };

            if (check_collision_rects(player_collider, tile_collider)) {
                if (attribs.vel.x > 0) {
                    attribs.pos.x = tile_pos.x - attribs.size.x - 0.01f;
                } else if (attribs.vel.x < 0) {
                    attribs.pos.x = tile_pos.x + BLOCK_SIZE + 0.01f;
                }
                attribs.vel.x = 0;
            }
        }
    }

    attribs.pos.y += attribs.vel.y * dt;
    attribs.ground = false;

    idx = (int)attribs.pos.x / (CHUNK_SIZE * BLOCK_SIZE);

    for (int i = -1; i < 2; i++) {
        int current_idx = idx + i;
        if (current_idx < (int)chunk_get_leftmost_idx() || current_idx > (int)chunk_get_rightmost_idx()) {
            continue;
        }

        chunk_t **chunk_ptr = hm_get(chunk_get_chunkmap(), current_idx);
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

            if (attribs.pos.x + attribs.size.x <= tile_pos.x || attribs.pos.x >= tile_pos.x + BLOCK_SIZE) {
                continue;
            }

            rect_collider_t player_collider = {
                .pos = attribs.pos, .size = attribs.size
            };
            rect_collider_t tile_collider = {
                .pos = tile_pos, .size = VEC2F(BLOCK_SIZE, BLOCK_SIZE)
            };

            if (check_collision_rects(player_collider, tile_collider)) {
                if (attribs.vel.y > 0) {
                    attribs.pos.y = tile_pos.y - attribs.size.y - 0.01f;
                    attribs.ground = true;
                } else if (attribs.vel.y < 0) {
                    attribs.pos.y = tile_pos.y + BLOCK_SIZE + 0.01f; 
                }
                attribs.vel.y = 0;
            }
        }
    }
}

#pragma endregion

void player_update(item_drop_t **drops) {
    assert(player);
    assert(chunks);
    assert(drops);

    player_handle_controls(drops);
    player_move_and_collide();
}

uint32_t player_set_spawn_point(fnl_state *terrain) {
    uint32_t spawn_chunk_x = (uint32_t)(MAP_SIZE * 0.5f);
    float noise = (fnlGetNoise2D(terrain, (float)((uint32_t)(spawn_chunk_x * CHUNK_SIZE)), 0) + 1.0f) * 0.5f;
    uint32_t height = MIN_TERRAIN_HEIGHT + (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));
    attribs.pos.x = (float)spawn_chunk_x * CHUNK_SIZE * BLOCK_SIZE;
    attribs.pos.y = (float)(MAX_CHUNK_HEIGHT - height - 1) * BLOCK_SIZE;
    return spawn_chunk_x;
}

#pragma region UI

void player_draw_inventory() {
    if (inventory.enabled) {
        texture_t *inventory_texture = textures_get_ui_texture(TEXTURE_INVENTORY);
        texture_t *hotbar_arrow = textures_get_ui_texture(TEXTURE_HOTBAR_ARROW);
        begin_draw(SHAPE_2D_UNLIT, false);
        draw_rect(VEC2F(0, 0), get_screen_size(), RGBA(0, 0, 0, 125), NO_ROTATION);
        begin_draw(TEXTURE_2D_UNLIT, false);
        vec2f size = VEC2F(inventory_texture->size.x * 4, inventory_texture->size.y * 4);
        vec2f pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f), (get_screen_height() * 0.5f) - (size.y * 0.5f));
        draw_texture2D(inventory_texture, pos, size, WHITE, NO_ROTATION);

        vec2f first_hotbar_slot = VEC2F(pos.x + (4 * 4), pos.y + size.y - (24 * 4));
        for (uint32_t i = 0; i < 9; i++) {
            if (inventory.hotbar[i].count == 0 ||
                inventory.hotbar[i].item == ITEM_NONE) {
                continue;
            }
            vec2f arrow_size = VEC2F(hotbar_arrow->size.x * 4, hotbar_arrow->size.y * 4);
            vec2f item_size = VEC2F(textures_get_item_texture(inventory.hotbar[i].item)->size.x * 4, 
                                    textures_get_item_texture(inventory.hotbar[i].item)->size.y * 4);
            vec2f slot_pos = VEC2F(first_hotbar_slot.x + (arrow_size.x * 0.5f) - (item_size.x * 0.5f) + ((18 * 4) * i), first_hotbar_slot.y);
            draw_texture2D(textures_get_item_texture(inventory.hotbar[i].item), slot_pos, item_size, WHITE, NO_ROTATION);
        }

        vec2f first_inventory_slot = VEC2F(pos.x + (4 * 4), pos.y + (84 * 4));
        for (uint32_t y = 0; y < 3; y++) {
            for (uint32_t x = 0; x < 9; x++) {
                if (inventory.slots[(y * 9) + x].count == 0 ||
                    inventory.slots[(y * 9) + x].item == ITEM_NONE) {
                    continue;
                }
                vec2f arrow_size = VEC2F(hotbar_arrow->size.x * 4, hotbar_arrow->size.y * 4);
                vec2f item_size = VEC2F(textures_get_item_texture(inventory.slots[(y * 9) + x].item)->size.x * 4, 
                                        textures_get_item_texture(inventory.slots[(y * 9) + x].item)->size.y * 4);
                vec2f slot_pos = VEC2F(first_inventory_slot.x + (arrow_size.x * 0.5f) - (item_size.x * 0.5f) + ((18 * 4) * x),
                                       first_inventory_slot.y + ((18 * 4) * y));
                draw_texture2D(textures_get_item_texture(inventory.slots[(y * 9) + x].item), slot_pos, item_size, WHITE, NO_ROTATION);
            }
        }

        begin_draw(TEXT, false);

        for (uint32_t i = 0; i < 9; i++) {
            vec2f arrow_size = VEC2F(hotbar_arrow->size.x * 4, hotbar_arrow->size.y * 4);
            vec2f item_size = VEC2F(textures_get_item_texture(inventory.hotbar[i].item)->size.x * 4, 
                                    textures_get_item_texture(inventory.hotbar[i].item)->size.y * 4);
            vec2f slot_pos = VEC2F(first_hotbar_slot.x + (arrow_size.x * 0.5f) - (item_size.x * 0.5f) + ((18 * 4) * i), first_hotbar_slot.y);
            if (inventory.hotbar[i].count <= 1 || inventory.hotbar[i].item == ITEM_NONE) {
                continue;
            }
    
            vec2f text_pos = VEC2F(slot_pos.x + arrow_size.x - (12 * 4), slot_pos.y + arrow_size.y - (12 * 4));
            draw_text_shadow(textures_get_font(), text_pos, 0.7f, WHITE, VEC2F(4, 4), BLACK, "%d", inventory.hotbar[i].count);
        }

        for (uint32_t y = 0; y < 3; y++) {
            for (uint32_t x = 0; x < 9; x++) {
                vec2f arrow_size = VEC2F(hotbar_arrow->size.x * 4, hotbar_arrow->size.y * 4);
                vec2f item_size = VEC2F(textures_get_item_texture(inventory.slots[(y * 9) + x].item)->size.x * 4, 
                                        textures_get_item_texture(inventory.slots[(y * 9) + x].item)->size.y * 4);
                float offset_y = 20.0f;
                vec2f slot_pos = VEC2F(first_inventory_slot.x + (arrow_size.x * 0.5f) - (item_size.x * 0.5f) + ((18 * 4) * x), 
                                       first_inventory_slot.y + ((18 * 4) * y));
                if (inventory.slots[(y * 9) + x].count <= 1 || inventory.slots[(y * 9) + x].item == ITEM_NONE) {
                    continue;
                }
    
                vec2f text_pos = VEC2F(slot_pos.x + arrow_size.x - (12 * 4), slot_pos.y + arrow_size.y - (12 * 4));
                draw_text_shadow(textures_get_font(), text_pos, 0.7f, WHITE, VEC2F(4, 4), BLACK, "%d", inventory.slots[(y * 9) + x].count);
            }
        }

        if (item_dragged.item != ITEM_NONE && item_dragged.count != 0) {
            begin_draw(TEXTURE_2D_UNLIT, false);
            vec2f dragged_size_half = VEC2F((textures_get_item_texture(item_dragged.item)->size.x * 4) * 0.5f, 
                                            (textures_get_item_texture(item_dragged.item)->size.y * 4) * 0.5f);
            draw_texture2D(textures_get_item_texture(item_dragged.item), vec2f_sub(get_mouse_pos(), dragged_size_half), 
                           vec2f_float_mul(textures_get_item_texture(item_dragged.item)->size, 4), WHITE, NO_ROTATION);
           
            if (item_dragged.count > 1) {
                begin_draw(TEXT, false);
                vec2f arrow_size = VEC2F(hotbar_arrow->size.x * 4, hotbar_arrow->size.y * 4);
                vec2f text_pos = VEC2F(get_mouse_pos().x + arrow_size.x - (20 * 4), get_mouse_pos().y + arrow_size.y - (20 * 4));
                draw_text_shadow(textures_get_font(), text_pos, 0.7f, WHITE, VEC2F(4, 4), BLACK, "%d", item_dragged.count);
            }
        }
    }
}

void player_draw_ui() {
    if (!inventory.enabled) {
        texture_t *hotbar = textures_get_ui_texture(TEXTURE_HOTBAR);
        texture_t *hotbar_arrow = textures_get_ui_texture(TEXTURE_HOTBAR_ARROW);

        begin_draw(TEXTURE_2D_UNLIT, false);

        vec2f size = VEC2F(hotbar->size.x * 4, hotbar->size.y * 4);
        float offset_y = 20.0f;
        vec2f hotbar_pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f), get_screen_height() - size.y - offset_y);
        draw_texture2D(hotbar, hotbar_pos, size, WHITE, NO_ROTATION);

        vec2f arrow_size = VEC2F(hotbar_arrow->size.x * 4, hotbar_arrow->size.y * 4);
        vec2f arrow_pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f) + ((arrow_size.x - (4 * 4)) * inventory.hotbar_selected) - 4,
                                get_screen_height() - size.y - offset_y - 4);
        draw_texture2D(hotbar_arrow, arrow_pos, arrow_size, WHITE, NO_ROTATION);

        for (uint32_t i = 0; i < 9; i++) {
            vec2f slot_pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f) + ((arrow_size.x - (4 * 4)) * i) - 4, 
                                   get_screen_height() - size.y - offset_y - 4);
            if (inventory.hotbar[i].count == 0 || inventory.hotbar[i].item == ITEM_NONE) {
                continue;
            }
            vec2f item_size = VEC2F(textures_get_item_texture(inventory.hotbar[i].item)->size.x * 4, 
                                    textures_get_item_texture(inventory.hotbar[i].item)->size.y * 4);
            draw_texture2D(textures_get_item_texture(inventory.hotbar[i].item), 
                           VEC2F(slot_pos.x + (arrow_size.x * 0.5f) - (item_size.x * 0.5f), slot_pos.y + (arrow_size.y * 0.5f) - (item_size.y * 0.5f)),
                           item_size, WHITE, NO_ROTATION);
        }

        tilemap_begin_editing(&stats.icons_bg);
        float icon_offset = 10.0f;
        for (uint32_t i = 0; i < 10; i++) {
            vec2f icon_pos = VEC2F(hotbar_pos.x + ((ICON_PIXEL_SIZE) * 4 * i), hotbar_pos.y - (ICON_PIXEL_SIZE * 4) - icon_offset);
            tilemap_add_tile(&stats.icons_bg, icon_pos, VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HEART_BG);
        }

        vec2f last_heart_pos = VEC2F(hotbar_pos.x + ((ICON_PIXEL_SIZE) * 4 * 9), hotbar_pos.y - (ICON_PIXEL_SIZE * 4) - icon_offset);

        float hunger_bar_offset = 10.0f * 4;
        for (uint32_t i = 0; i < 10; i++) {
            vec2f icon_pos = VEC2F(last_heart_pos.x + (ICON_PIXEL_SIZE * 4) + ((ICON_PIXEL_SIZE - 1) * 4 * i) + hunger_bar_offset, last_heart_pos.y);
            tilemap_add_tile(&stats.icons_bg, icon_pos, VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HUNGER_BG);
        }

        tilemap_begin_editing(&stats.icons);
        for (uint32_t i = 0; i < (uint32_t)(stats.health * 0.5f); i++) {
            vec2f icon_pos = VEC2F(hotbar_pos.x + ((ICON_PIXEL_SIZE) * 4 * i), hotbar_pos.y - (ICON_PIXEL_SIZE * 4) - icon_offset);
            tilemap_add_tile(&stats.icons, icon_pos, VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HEART);
        }
        int health_left = (int)stats.health % 2;
        if (health_left == 1) {
            vec2f icon_pos = VEC2F(hotbar_pos.x + ((ICON_PIXEL_SIZE) * 4 * (uint32_t)(stats.health * 0.5f)), 
                                   hotbar_pos.y - (ICON_PIXEL_SIZE * 4) - icon_offset);
            tilemap_add_tile(&stats.icons, icon_pos, VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HEART_HALF);
        }

        for (uint32_t i = 0; i < (uint32_t)(stats.hunger * 0.5f); i++) {
            vec2f icon_pos = VEC2F(last_heart_pos.x + (ICON_PIXEL_SIZE * 4) + ((ICON_PIXEL_SIZE - 1) * 4 * i) + hunger_bar_offset, last_heart_pos.y);
            tilemap_add_tile(&stats.icons, icon_pos, VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HUNGER);
        }
        int hunger_left = (int)stats.health % 2;
        if (hunger_left == 1) {
            vec2f icon_pos = VEC2F(last_heart_pos.x + (ICON_PIXEL_SIZE * 4) + ((ICON_PIXEL_SIZE - 1) * 4 * (uint32_t)(stats.hunger * 0.5f)) +
                                   hunger_bar_offset, last_heart_pos.y);
            tilemap_add_tile(&stats.icons, icon_pos, VEC2F(ICON_PIXEL_SIZE * 4, ICON_PIXEL_SIZE * 4), ICON_HUNGER_HALF);
        }

        tilemap_draw(&stats.icons_bg, WHITE);
        tilemap_draw(&stats.icons, WHITE);

        begin_draw(TEXT, false);

        for (uint32_t i = 0; i < 9; i++) {
            vec2f slot_pos = VEC2F((get_screen_width() * 0.5f) - (size.x * 0.5f) + ((arrow_size.x - (4 * 4)) * i) - 4,
                                   get_screen_height() - size.y - offset_y - 4);
            if (inventory.hotbar[i].count <= 1 || inventory.hotbar[i].item == ITEM_NONE) {
                continue;
            }

            vec2f text_pos = VEC2F(slot_pos.x + arrow_size.x - (9 * 4), slot_pos.y + arrow_size.y - (9 * 4));
            draw_text_shadow(textures_get_font(), text_pos, 0.7f, WHITE, VEC2F(4, 4), BLACK, "%d", inventory.hotbar[i].count);
        }
    }
}

#pragma endregion
