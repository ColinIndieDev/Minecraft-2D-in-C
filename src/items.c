#include "items.h"
#include <cpl/cpl.h>
#include <cpstd/cpmath.h>

#define MAX_ANIM_OFFSET 10

void drop_item(item_drop *drop, vec2f pos, item_types type) {
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

b8 item_placable(item_types type) {
    if (type == ITEM_GRASS_BLOCK || type == ITEM_DIRT || type == ITEM_STONE ||
        type == ITEM_SAND || type == ITEM_BEDROCK || type == ITEM_FLOWER_ROSE ||
        type == ITEM_SUGAR_CANE || type == ITEM_OAK_LOG ||
        type == ITEM_OAK_LEAVES) {
        return true;
    }
    return false;
}

void update_drop(item_drop *drop, chunk *chunks) {
    drop->vel.y += drop->gravity * get_dt();
    if (drop->vel.y > drop->max_fall_speed) {
        drop->vel.y = drop->max_fall_speed;
    }

    drop->collider_pos.y += drop->vel.y * get_dt();
    drop->ground = false;
    i32 idx = (i32)drop->collider_pos.x / (CHUNK_SIZE * BLOCK_SIZE);
    for (u32 t = 0; t < chunks[idx].tiles.renderer.count / 6; t++) {
        if (!chunks[idx].tiles.renderer.collidable[t]) {
            continue;
        }
        vec2f tile_pos =
            VEC2F(chunks[idx].tiles.renderer.vertices[(u64)t * 6].x,
                  chunks[idx].tiles.renderer.vertices[(u64)t * 6].y);
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
    }
    f32 offset = 0.0f;
    if (drop->ground) {
        offset = cpm_sinf(get_time() * 2) * MAX_ANIM_OFFSET;
    }
    drop->pos = VEC2F(drop->collider_pos.x + (drop->size.x * 0.5f),
                      drop->collider_pos.y + (drop->size.y * 0.5f) + offset);
}

void draw_drop(item_drop *drop, texture *item_textures) {
    color c = WHITE;
    if (get_time() >= drop->timer + ITEM_DROP_LIFETIME - 10.0f) {
        f32 dt = drop->timer + ITEM_DROP_LIFETIME - get_time();
        f32 scale = (f32)(dt / 10.0f);

        c.a = scale * 255.0f;
    }
    draw_texture2D(&item_textures[drop->type], VEC2F(drop->pos.x, drop->pos.y),
                   drop->size, c, 0);
}
