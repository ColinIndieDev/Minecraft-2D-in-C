#include "items.h"

item_data_t item_data[ITEM_TYPES] = {
    {.placable = false},                                                // None
    {.placable = true, .tex_path = ITEM_TEX_PATH "grass_block.png"},    // Grass Block
    {.placable = true, .tex_path = ITEM_TEX_PATH "dirt.png"},           // Dirt 
    {.placable = true, .tex_path = ITEM_TEX_PATH "stone.png"},          // Stone 
    {.placable = true, .tex_path = ITEM_TEX_PATH "sand.png"},           // Sand
    {.placable = true, .tex_path = ITEM_TEX_PATH "bedrock.png"},        // Bedrock
    {.placable = true, .tex_path = ITEM_TEX_PATH "rose.png"},           // Rose
    {.placable = true, .tex_path = ITEM_TEX_PATH "sugar_cane.png"},     // Sugar Cane
    {.placable = true, .tex_path = ITEM_TEX_PATH "oak_log.png"},        // Oak Log
    {.placable = true, .tex_path = ITEM_TEX_PATH "oak_leaves.png"},     // Oak Leaves
    {.placable = true, .tex_path = ITEM_TEX_PATH "gravel.png"},         // Gravel
    {.placable = true, .tex_path = ITEM_TEX_PATH "cobblestone.png"},    // Cobblestone
    {.placable = true, .tex_path = ITEM_TEX_PATH "oak_planks.png"},     // Oak Planks
    {.placable = true, .tex_path = ITEM_TEX_PATH "coal_ore.png"},       // Coal Ore
    {.placable = true, .tex_path = ITEM_TEX_PATH "iron_ore.png"},       // Iron Ore
    {.placable = true, .tex_path = ITEM_TEX_PATH "diamond_ore.png"},    // Diamond Ore
    {.placable = false, .tex_path = ITEM_TEX_PATH "coal.png"},          // Coal
    {.placable = false, .tex_path = ITEM_TEX_PATH "diamond.png"},       // Diamond
    {.placable = true, .tex_path = ITEM_TEX_PATH "dandelion.png"},      // Dandelion
    {.placable = true, .tex_path = ITEM_TEX_PATH "houstonia.png"},      // Houstonia
    {.placable = true, .tex_path = ITEM_TEX_PATH "oxeye_daisy.png"},    // Oxeye Daisy
    {.placable = true, .tex_path = ITEM_TEX_PATH "allium.png"},         // Allium
    {.placable = true, .tex_path = ITEM_TEX_PATH "grass.png"},          // Grass
};

void item_drop_item(item_drop *drop, item_types type, vec2f pos) {
    drop->collider_pos = pos;
    drop->collider_size = VEC2F(50, 50);
    drop->size =
        VEC2F(drop->collider_size.x * 0.5f, drop->collider_size.y * 0.5f);
    drop->pos =
        VEC2F(pos.x + (drop->size.x * 0.5f), pos.y + (drop->size.y * 0.5f));
    drop->vel = VEC2F(0, 0);
    drop->gravity = 750.0f;
    drop->max_fall_speed = 1100.0f;
    drop->type = type;
    drop->ground = false;
    drop->timer = get_time();
}

void item_update_drop_collision(item_drop *drop, chunk_map *chunks) {
    drop->vel.y += drop->gravity * get_dt();
    if (drop->vel.y > drop->max_fall_speed) {
        drop->vel.y = drop->max_fall_speed;
    }

    drop->collider_pos.y += drop->vel.y * get_dt();
    drop->ground = false;
    i32 idx = (i32)drop->collider_pos.x / (CHUNK_SIZE * BLOCK_SIZE);
    for (u32 t = 0; t < (*chunk_map_get(chunks, idx))->tiles.renderer.count / 6; t++) {
        if (!(*chunk_map_get(chunks, idx))->tiles.renderer.collidable[t]) {
            continue;
        }
        vec2f tile_pos =
            VEC2F((*chunk_map_get(chunks, idx))->tiles.renderer.vertices[(u64)t * 6].x,
                  (*chunk_map_get(chunks, idx))->tiles.renderer.vertices[(u64)t * 6].y);
        if (drop->collider_pos.x + drop->size.x <= tile_pos.x) {
            continue;
        }
        if (drop->collider_pos.x >= tile_pos.x + BLOCK_SIZE) {
            continue;
        }
        rect_collider drop_collider = {.pos = drop->collider_pos,
                                       .size = drop->collider_size};
        rect_collider tile_collider = {.pos = tile_pos,
                                       .size = VEC2F(BLOCK_SIZE, BLOCK_SIZE)};

        if (check_collision_rects(drop_collider, tile_collider)) {
            if (drop->vel.y > 0) {
                drop->collider_pos.y = tile_pos.y - drop->collider_size.y;
                drop->ground = true;
            } else if (drop->vel.y < 0) {
                drop->collider_pos.y = tile_pos.y + BLOCK_SIZE +
                                       0.1f; // Prevent glitching through
                                             // blocks if jumping into them
                                             // below by slightly offsetting
            }
            drop->vel.y = 0;
        }

        if (check_collision_vec2f_rect(drop->collider_pos, tile_collider)) {
            if (drop->collider_pos.x <
                tile_pos.x + (tile_collider.size.x * 0.5f)) {
                drop->collider_pos.x = tile_pos.x - drop->collider_size.x;
            } else {
                drop->collider_pos.x = tile_pos.x + tile_collider.size.x;
            }
        }
    }
}

void item_update_anim(item_drop *drop) {
    f32 offset = 0.0f;
    if (drop->ground) {
        offset = cpm_sinf(get_time() * 2) * MAX_ANIM_OFFSET;
    }
    drop->pos = VEC2F(drop->collider_pos.x + (drop->size.x * 0.5f),
                      drop->collider_pos.y + (drop->size.y * 0.5f) + offset);
}

void item_update_drop(item_drop *drop, chunk_map *chunks) {
    item_update_drop_collision(drop, chunks);
    item_update_anim(drop);
}

void item_draw_drop(item_drop *drop, texture *item_textures) {
    color c = WHITE;
    if (get_time() >= drop->timer + ITEM_DROP_LIFETIME - 10.0f) {
        f32 dt = drop->timer + ITEM_DROP_LIFETIME - get_time();
        f32 scale = (f32)(dt / 10.0f);

        c.a = scale * 255.0f;
    }
    draw_texture2D(&item_textures[drop->type], VEC2F(drop->pos.x, drop->pos.y),
                   drop->size, c, 0);
}
