#define CPL_IMPL
#include "core.h"

#define FNL_IMPL
#include "game/blocks.h"
#include "game/chunk.h"
#include "game/items.h"
#include "game/player.h"
#include "game/textures.h"

#include <cpstd/rand.h>

item_drop_t *item_drops = NULL;

#pragma region init() Helper

void init_player() {
    item_drops = vec_init(item_drops, 10);
    for (uint32_t i = 0; i < 9; i++) {
        player_get_inventory_properties()->hotbar[i] = (slot_t){ITEM_NONE, 0};
    }
    tilemap_create(&player_get_stats_properties()->icons, VEC2F(9, 9));
    tilemap_load_texture(&player_get_stats_properties()->icons, "assets/images/gui/icons.png", FILTER_NEAREST);
    tilemap_create(&player_get_stats_properties()->icons_bg, VEC2F(9, 9));
    tilemap_load_texture(&player_get_stats_properties()->icons_bg, "assets/images/gui/icons.png", FILTER_NEAREST);
}

#pragma endregion

void init() {
    pcg_rand_seed();
#ifndef __EMSCRIPTEN__
    window_init(800, 800, "Hello CPL", OPENGL_3_3);
#else
    window_init(800, 800, "Hello CPL", OPENGL_3_0);
#endif
    enable_vsync(false);

    chunk_init_threads();
    textures_load_resources();
    init_player();
    chunk_gen_init();
}

#pragma region main_loop() Helper

void draw_info() {
    draw_text_shadow(textures_get_font(), VEC2F(10, 10), 0.7f, WHITE, VEC2F(3, 3), BLACK, "FPS: %d", get_fps());
    draw_text_shadow(textures_get_font(), VEC2F(10, 60), 0.7f, WHITE, VEC2F(3, 3), BLACK, "Seed: %d", chunk_get_map_noise()->terrain.seed);
    draw_text_shadow(textures_get_font(), VEC2F(10, 110), 0.7f, WHITE, VEC2F(3, 3), BLACK, "X: %d Y: %d (Chunk: %d)",
            (int)player_get_attribs_properties()->pos.x / BLOCK_SIZE, 
            MAX_CHUNK_HEIGHT - (int)(player_get_attribs_properties()->pos.y / BLOCK_SIZE),
            (int)player_get_attribs_properties()->pos.x / BLOCK_SIZE / CHUNK_SIZE);
    draw_text_shadow(textures_get_font(), VEC2F(10, 160), 0.7f, WHITE, VEC2F(3, 3), BLACK, "Chunks generated: %d", hm_size(chunk_get_chunkmap()));
}

#pragma endregion

void main_loop() {
    update();

    player_update(&item_drops);

    clear_background(LIGHT_BLUE);
    begin_draw(TEXTURE_2D_UNLIT, true);

    chunk_calc_chunks_to_render();
    chunk_gen_chunks_req();
    chunk_draw_chunks();

    foreach_vec(drop, item_drops) {
        items_draw_drop(drop);
    }

    begin_draw(SHAPE_2D_UNLIT, true);

    player_draw();

    if (!vec2f_cmp(player_get_mining_properties()->block, VEC2F(-1, -1))) {
        draw_rect(player_get_mining_properties()->block,
                  VEC2F((get_time() - player_get_mining_properties()->timer) / player_get_mining_properties()->block_dt * BLOCK_SIZE, BLOCK_SIZE),
                  RGBA(255, 255, 255, 125), NO_ROTATION);
    }

    begin_draw(TEXTURE_2D_UNLIT, true);

    chunk_calc_chunks_to_render();
    chunk_draw_chunks_passable();

    begin_draw(TEXTURE_2D_UNLIT, false);

    player_draw_ui();
    player_draw_inventory();

    begin_draw(TEXT, false);

    draw_info();

    end_frame();
}

void terminate() {
    chunk_close_threads();
    window_close();
}
