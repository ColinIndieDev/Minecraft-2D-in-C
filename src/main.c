#include <cpstd/cpvec.h>
#include <stdio.h>
#define CPL_IMPLEMENTATION
#ifndef __EMSCRIPTEN__
#include <cpl/cpl.h>
#endif
#ifdef __EMSCRIPTEN__
#include "../external/cpl.h"
#endif
#define FNL_IMPL

#include "blocks.h"
#include "chunk.h"
#include "items.h"
#include "player.h"

#define CPRNG_IMPL
#ifndef __EMSCRIPTEN__
#include <cpstd/cprng.h>
#endif
#ifdef __EMSCRIPTEN__
#include "../cpstd/cprng.h"
#endif

block_data_t block_data[BLOCK_TYPES] = {
    {VEC2F(1, 0), 0.6f, false},  // Grass Block
    {VEC2F(0, 0), 0.6f, false},  // Dirt
    {VEC2F(2, 0), 10.0f, false}, // Stone
    {VEC2F(0, 1), 0.6f, false},  // Sand
    {VEC2F(1, 1), -1.0f, true},  // Bedrock
    {VEC2F(2, 1), -1.0f, true},  // Water
    {VEC2F(0, 2), 0.1f, false},  // Rose
    {VEC2F(1, 2), 0.3f, false},  // Sugar Cane
    {VEC2F(3, 0), 2.25f, false}, // Oak Log
    {VEC2F(3, 1), 0.3f, false},  // Oak Leaves

};
texture item_textures[ITEM_TYPES];
player_t player = {.pos = VEC2F(0, 0),
                   .size = VEC2F(0.5f * BLOCK_SIZE, 1.75f * BLOCK_SIZE),
                   .vel = VEC2F(0, 0),
                   .ground = true,
                   .jmp_force = 450.0f,
                   .gravity = 900.0f,
                   .move_speed = PLAYER_BASE_SPEED,
                   .max_fall_speed = 1100.0f,
                   .block_mining = VEC2F(-1, -1),
                   .block_mining_dt = 0.0f,
                   .block_mining_timer = 0.0f,
                   .hotbar_selected = 0};
font f;
map_noise_t map_noise;
chunk c[MAP_SIZE];
VEC_IMPL(item_drop, vec_item_drop)
vec_item_drop item_drops;

texture hotbar;
texture hotbar_arrow;

void init();
void main_loop();
void draw_ui();

int main(void) {
    init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while (!window_should_close()) {
        main_loop();
    }
#endif

    close_window();
}

void init() {
    cprng_rand_seed();
#ifndef __EMSCRIPTEN__
    init_window(800, 800, "Hello CPL", OPENGL_VER_3_3);
#else
    init_window(800, 800, "Hello CPL", OPENGL_VER_3_0);
#endif
    enable_vsync(false);
    create_font(&f, "assets/fonts/default.ttf", "default", FILTER_LINEAR);

    load_texture(&item_textures[ITEM_GRASS_BLOCK],
                 "assets/images/items/grass_block.png", FILTER_NEAREST);
    load_texture(&item_textures[ITEM_DIRT], "assets/images/items/dirt.png",
                 FILTER_NEAREST);
    load_texture(&item_textures[ITEM_STONE], "assets/images/items/stone.png",
                 FILTER_NEAREST);
    load_texture(&item_textures[ITEM_SAND], "assets/images/items/sand.png",
                 FILTER_NEAREST);
    load_texture(&item_textures[ITEM_BEDROCK],
                 "assets/images/items/bedrock.png", FILTER_NEAREST);
    load_texture(&item_textures[ITEM_FLOWER_ROSE],
                 "assets/images/items/rose.png", FILTER_NEAREST);
    load_texture(&item_textures[ITEM_SUGAR_CANE],
                 "assets/images/items/sugar_cane.png", FILTER_NEAREST);
    load_texture(&item_textures[ITEM_OAK_LOG],
                 "assets/images/items/oak_log.png", FILTER_NEAREST);
    load_texture(&item_textures[ITEM_OAK_LEAVES],
                 "assets/images/items/oak_leaves.png", FILTER_NEAREST);

    load_texture(&hotbar, "assets/images/gui/hotbar.png", FILTER_NEAREST);
    load_texture(&hotbar_arrow, "assets/images/gui/hotbar_arrow.png", FILTER_NEAREST);

    vec_item_drop_reserve(&item_drops, 10);

    gen_seed(&map_noise);

    for (u32 i = 0; i < 9; i++) {
        player.hotbar[i] = (slot){ITEM_NONE, 0};
    }

    // Temporarily, but it actually does not suck
    for (u32 i = 0; i < MAP_SIZE; i++) {
        chunk_gen(&c[i], VEC2F(i, 0), &map_noise, block_data);
        tilemap_check_collidable_tiles(&c[i].tiles,
                                       VEC2F(BLOCK_SIZE, BLOCK_SIZE));
    }
    set_spawn_point(&player, &map_noise.terrain);
}

void main_loop() {
    update();

    handle_controls(&player, c, block_data, &item_drops);
    move_and_collide(&player, c);

    FOREACH_VEC(item_drop, vec_item_drop, drop, &item_drops) {
        update_drop(drop, c);
    }
    u32 w = 0;
    for (u32 i = 0; i < item_drops.size; i++) {
        item_drop drop = item_drops.data[i];
        if (!(drop.timer + ITEM_DROP_LIFETIME <= get_time())) {
            item_drops.data[w++] = item_drops.data[i];
        }
    }
    item_drops.size = w;

    clear_background(LIGHT_BLUE);

    begin_draw(TEXTURE_2D_UNLIT, true);

    {
        f32 inv_block_scale = 1.0f / (CHUNK_SIZE * BLOCK_SIZE);

        i32 start_chunk =
            (i32)(get_cam_2D()->pos.x * (1.0f / (CHUNK_SIZE * BLOCK_SIZE)));
        i32 end_chunk =
            (i32)((get_cam_2D()->pos.x +
                   ((f32)get_screen_width() * (1.0f / get_cam_2D()->zoom))) *
                  (1.0f / (CHUNK_SIZE * BLOCK_SIZE)));

        for (i32 i = start_chunk; i <= end_chunk; i++) {
            if (i >= 0 && i < MAP_SIZE) {
                chunk_draw(&c[i]);
            }
        }
    }

    FOREACH_VEC(item_drop, vec_item_drop, drop, &item_drops) {
        draw_drop(drop, item_textures);
    }

    begin_draw(SHAPE_2D_UNLIT, true);

    draw_player(&player);

    if (!vec2f_cmp(player.block_mining, VEC2F(-1, -1))) {
        draw_rect(player.block_mining,
                  VEC2F((get_time() - player.block_mining_timer) /
                            player.block_mining_dt * BLOCK_SIZE,
                        BLOCK_SIZE),
                  RGBA(255, 255, 255, 125), 0);
    }

    begin_draw(TEXTURE_2D_UNLIT, true);

    {
        f32 inv_block_scale = 1.0f / (CHUNK_SIZE * BLOCK_SIZE);

        i32 start_chunk =
            (i32)(get_cam_2D()->pos.x * (1.0f / (CHUNK_SIZE * BLOCK_SIZE)));
        i32 end_chunk =
            (i32)((get_cam_2D()->pos.x +
                   ((f32)get_screen_width() * (1.0f / get_cam_2D()->zoom))) *
                  (1.0f / (CHUNK_SIZE * BLOCK_SIZE)));

        for (i32 i = start_chunk; i <= end_chunk; i++) {
            if (i >= 0 && i < MAP_SIZE) {
                chunk_draw_passable(&c[i]);
            }
        }
    }

    begin_draw(TEXTURE_2D_UNLIT, false);

    draw_gui(&player, &hotbar, &hotbar_arrow, item_textures, &f);

    begin_draw(TEXT, false);

    draw_ui();
    // display_details(&f);

    end_frame();
}

void draw_ui() {
    {
        char txt[100];
        snprintf(txt, sizeof(txt), "FPS: %d", get_fps());
        draw_text_shadow(&f, txt, VEC2F(10, 10), 0.7f, WHITE, VEC2F(3, 3),
                         BLACK);
    }
    {
        char txt[100];
        snprintf(txt, sizeof(txt), "Seed: %d", map_noise.terrain.seed);
        draw_text_shadow(&f, txt, VEC2F(10, 60), 0.7f, WHITE, VEC2F(3, 3),
                         BLACK);
    }
    {
        char txt[100];
        snprintf(txt, sizeof(txt), "X: %d Y: %d",
                 (i32)player.pos.x / BLOCK_SIZE,
                 MAX_CHUNK_HEIGHT - (i32)(player.pos.y / BLOCK_SIZE));
        draw_text_shadow(&f, txt, VEC2F(10, 110), 0.7f, WHITE, VEC2F(3, 3),
                         BLACK);
    }
    {
        char txt[100];
        snprintf(txt, sizeof(txt), "Chunk ID: %d",
                 (i32)player.pos.x / BLOCK_SIZE / CHUNK_SIZE);
        draw_text_shadow(&f, txt, VEC2F(10, 160), 0.7f, WHITE, VEC2F(3, 3),
                         BLACK);
    }
}
