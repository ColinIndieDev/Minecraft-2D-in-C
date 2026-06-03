#include <stdlib.h>
#define CPL_IMPLEMENTATION
#define CPRNG_IMPL
#ifndef __EMSCRIPTEN__
#include <cpl/cpl.h>
#include <cpstd/cprng.h>
#else
#include "../cpstd/cprng.h"
#include "../external/cpl.h"
#endif
#define FNL_IMPL

#include "blocks.h"
#include "chunk.h"
#include "items.h"
#include "player.h"
#include "textures.h"

EXTERN_BLOCKS_H_VARIABLES
EXTERN_ITEMS_H_VARIABLES
EXTERN_TEXTURES_H_VARIABLES

player_t player = {
    .attribs = {
        .pos = VEC2F(0, 0),
        .size = VEC2F(0.5f * BLOCK_SIZE, 1.75f * BLOCK_SIZE),
        .vel = VEC2F(0, 0),
        .ground = true,
        .jmp_force = 450.0f,
        .gravity = 900.0f,
        .move_speed = PLAYER_BASE_SPEED,
        .max_fall_speed = 1100.0f
    },
    .mining = {
        .block = VEC2F(-1, -1), 
        .block_dt = 0.0f, 
        .timer = 0.0f
    },
    .stats = {
        .health = 20, 
        .hunger = 20
    },
    .inventory = {
        .hotbar_selected = 0, 
        .enabled = false
    }
};

map_noise_t map_noise;
chunk c[MAP_SIZE];
VEC_IMPL(item_drop, vec_item_drop)
vec_item_drop item_drops;

void init_player();
void init_map();
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

// {{{ Init Helper Functions

void init_player() {
    vec_item_drop_reserve(&item_drops, 10);
    for (u32 i = 0; i < 9; i++) {
        player.inventory.hotbar[i] = (slot){ITEM_NONE, 0};
    }
    create_tilemap(&player.stats.icons, VEC2F(9, 9));
    tilemap_load_texture(&player.stats.icons, "assets/images/gui/icons.png",
                         FILTER_NEAREST);
    create_tilemap(&player.stats.icons_bg, VEC2F(9, 9));
    tilemap_load_texture(&player.stats.icons_bg, "assets/images/gui/icons.png",
                         FILTER_NEAREST);
}

void init_map() {
    chunk_gen_seed(&map_noise);

    // Temporarily, but it actually does not suck
    for (u32 i = 0; i < MAP_SIZE; i++) {
        chunk_gen(&c[i], &map_noise, block_data, VEC2F(i, 0));
        tilemap_check_collidable_tiles(&c[i].tiles,
                                       VEC2F(BLOCK_SIZE, BLOCK_SIZE));
    }
    player_set_spawn_point(&player, &map_noise.terrain);
}

// }}}

void init() {
    cprng_rand_seed();
#ifndef __EMSCRIPTEN__
    init_window(800, 800, "Hello CPL", OPENGL_VER_3_3);
#else
    init_window(800, 800, "Hello CPL", OPENGL_VER_3_0);
#endif
    enable_vsync(false);

    textures_load_resources();
    init_player();
    init_map();
}

void main_loop() {
    update();

    player_update(&player, c, block_data, &item_drops);

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
        item_draw_drop(drop, item_textures);
    }

    begin_draw(SHAPE_2D_UNLIT, true);

    player_draw(&player);

    if (!vec2f_cmp(player.mining.block, VEC2F(-1, -1))) {
        draw_rect(player.mining.block,
                  VEC2F((get_time() - player.mining.timer) /
                            player.mining.block_dt * BLOCK_SIZE,
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

    player_draw_gui(&player, &hotbar, &hotbar_arrow, item_textures, &f);
    player_draw_inventory(&player, &inventory, item_textures, &hotbar_arrow,
                          &f);

    begin_draw(TEXT, false);

    // draw_ui();
    display_details(&f);

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
                 (i32)player.attribs.pos.x / BLOCK_SIZE,
                 MAX_CHUNK_HEIGHT - (i32)(player.attribs.pos.y / BLOCK_SIZE));
        draw_text_shadow(&f, txt, VEC2F(10, 110), 0.7f, WHITE, VEC2F(3, 3),
                         BLACK);
    }
}
