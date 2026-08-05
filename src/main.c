#define CPL_IMPL
#include <cpl/cpl.h>
#include <cpstd/hashmap.h>
#include <cpstd/rand.h>

#define FNL_IMPL

#include "blocks.h"
#include "chunk.h"
#include "items.h"
#include "player.h"
#include "textures.h"

EXTERN_CHUNK_H_VARIABLES
EXTERN_BLOCKS_H_VARIABLES
EXTERN_ITEMS_H_VARIABLES
EXTERN_TEXTURES_H_VARIABLES

// A mf clanker told me I need this but I do not trust it + it does in fact work
// without the mutex but I guess I will leave this as an option in case it is
// right (I surely hope not)
#define USE_CHUNK_MAP_MUTEX false

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

uint32_t left_most_chunk = 0;
uint32_t right_most_chunk = 0;
map_noise_t map_noise;
chunk_entry *chunks = NULL;
pthread_mutex_t chunk_map_mutex = PTHREAD_MUTEX_INITIALIZER;
item_drop_t *item_drops = NULL;
chunk_t **chunk_gen_queue = NULL;
worker_data_t chunk_gen_worker_data = {
    .map_noise = &map_noise,
    .block_data = block_data
};

void init_player();
void init_map();
void init();
void main_loop();
void draw_info();

int main(void) {
    init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while (!window_should_close()) {
        main_loop();
    }
#endif

    chunk_close_threads(&chunk_gen_worker_data);
    pthread_mutex_destroy(&chunk_map_mutex);
    window_close();
}

// {{{ Init Helper Functions

void init_player() {
    item_drops = vec_init(item_drops, 10);
    for (uint32_t i = 0; i < 9; i++) {
        player.inventory.hotbar[i] = (slot_t){ITEM_NONE, 0};
    }
    tilemap_create(&player.stats.icons, VEC2F(9, 9));
    tilemap_load_texture(&player.stats.icons, "assets/images/gui/icons.png",
                         FILTER_NEAREST);
    tilemap_create(&player.stats.icons_bg, VEC2F(9, 9));
    tilemap_load_texture(&player.stats.icons_bg, "assets/images/gui/icons.png",
                         FILTER_NEAREST);
}

void init_map() {
    chunks = hm_init(chunks, 10);

    chunk_gen_seed(&map_noise);

    uint32_t spawn_chunk_x = player_set_spawn_point(&player, &map_noise.terrain);
    left_most_chunk = spawn_chunk_x - 1;
    right_most_chunk = spawn_chunk_x + 1;
    for (uint32_t i = left_most_chunk; i <= right_most_chunk; i++) {
#if USE_CHUNK_MAP_MUTEX
        pthread_mutex_lock(&chunk_map_mutex);
#endif
        chunk_t *new_chunk = malloc(sizeof(chunk_t));
        new_chunk->pos = VEC2F(i, 0);
        new_chunk->ready = false;
        hm_put(chunks, i, new_chunk);
#if USE_CHUNK_MAP_MUTEX
        pthread_mutex_unlock(&chunk_map_mutex);
#endif
        chunk_gen_gl(new_chunk);
        queue_push(chunk_gen_queue, new_chunk);
        pthread_cond_signal(&chunk_gen_cond);
    }
}

// }}}

void init() {
    pcg_rand_seed();
#ifndef __EMSCRIPTEN__
    window_init(800, 800, "Hello CPL", OPENGL_VER_3_3);
#else
    window_init(800, 800, "Hello CPL", OPENGL_VER_3_0);
#endif
    enable_vsync(false);

    chunk_gen_queue = queue_init(chunk_gen_queue, 10);
    chunk_gen_worker_data.queue = &chunk_gen_queue;

    chunk_init_threads(&chunk_gen_worker_data);
    textures_load_resources();
    init_player();
    init_map();
}

void main_loop() {
    update();

    player_update(&player, chunks, block_data, &item_drops, left_most_chunk, right_most_chunk, &inventory, item_textures, &hotbar_arrow);

    clear_background(LIGHT_BLUE);

    begin_draw(TEXTURE_2D_UNLIT, true);

    {
        float inv_block_scale = 1.0f / (CHUNK_SIZE * BLOCK_SIZE);

        int start_chunk = (int)(get_cam_2D()->pos.x * (1.0f / (CHUNK_SIZE * BLOCK_SIZE)));
        int end_chunk =
            (int)((get_cam_2D()->pos.x +
                   ((float)get_screen_width() * (1.0f / get_cam_2D()->zoom))) *
                  (1.0f / (CHUNK_SIZE * BLOCK_SIZE)));

        if (start_chunk < 0) {
            start_chunk = 0;
        }
        if (end_chunk >= MAP_SIZE) {
            end_chunk = MAP_SIZE - 1;
        }

        for (int i = start_chunk; i <= end_chunk; i++) {
#if USE_CHUNK_MAP_MUTEX
            pthread_mutex_lock(&chunk_map_mutex);
#endif
            if (i >= 0 && i < MAP_SIZE) {
                chunk_t *existing_chunk = NULL;
                if (hm_get(chunks, i)) {
                    existing_chunk = *hm_get(chunks, i);
                }

                if (!existing_chunk) {
                    chunk_t *new_chunk = malloc(sizeof(chunk_t));
                    new_chunk->pos = VEC2F(i, 0);
                    new_chunk->ready = false;
                    hm_put(chunks, i, new_chunk);
                    if (new_chunk != NULL) {
                        chunk_gen_gl(new_chunk);
                        queue_push(chunk_gen_queue, new_chunk);
                        pthread_cond_signal(&chunk_gen_cond);
                    }
#if USE_CHUNK_MAP_MUTEX
                    pthread_mutex_unlock(&chunk_map_mutex);
#endif
                    if (i < left_most_chunk) {
                        left_most_chunk = i;
                    }
                    if (i > right_most_chunk) {
                        right_most_chunk = i;
                    }
                } else {
#if USE_CHUNK_MAP_MUTEX
                    pthread_mutex_unlock(&chunk_map_mutex);
#endif
                }
            } else {
#if USE_CHUNK_MAP_MUTEX
                pthread_mutex_unlock(&chunk_map_mutex);
#endif
            }
        }

        for (int i = start_chunk; i <= end_chunk; i++) {
            if (i >= left_most_chunk && i <= right_most_chunk) {
                chunk_t **c_ptr = hm_get(chunks, i);
                chunk_t *c = c_ptr ? *c_ptr : NULL;
                if (c && c->ready) {
                    chunk_draw(c);
                }
            }
        }
    }

    foreach_vec(drop, item_drops) {
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
        float inv_block_scale = 1.0f / (CHUNK_SIZE * BLOCK_SIZE);

        int start_chunk =
            (int)(get_cam_2D()->pos.x * (1.0f / (CHUNK_SIZE * BLOCK_SIZE)));
        int end_chunk =
            (int)((get_cam_2D()->pos.x +
                   ((float)get_screen_width() * (1.0f / get_cam_2D()->zoom))) *
                  (1.0f / (CHUNK_SIZE * BLOCK_SIZE)));

        if (start_chunk < 0) {
            start_chunk = 0;
        }
        if (end_chunk >= MAP_SIZE) {
            end_chunk = MAP_SIZE - 1;
        }

        for (int i = start_chunk; i <= end_chunk; i++) {
            if (i >= left_most_chunk && i <= right_most_chunk) {
                chunk_t **c_ptr = hm_get(chunks, i);
                chunk_t *c = c_ptr ? *c_ptr : NULL;
                if (c && c->ready) {
                    chunk_draw_passable(c);
                }
            }
        }
    }

    begin_draw(TEXTURE_2D_UNLIT, false);

    player_draw_ui(&player, &hotbar, &hotbar_arrow, item_textures, &f);
    player_draw_inventory(&player, &inventory, item_textures, &hotbar_arrow,
                          &f);

    begin_draw(TEXT, false);

    draw_info();

    end_frame();
}

void draw_info() {
    draw_text_shadow(&f, VEC2F(10, 10), 0.7f, WHITE, VEC2F(3, 3), BLACK, "FPS: %d", get_fps());
    draw_text_shadow(&f, VEC2F(10, 60), 0.7f, WHITE, VEC2F(3, 3), BLACK, "Seed: %d", map_noise.terrain.seed);
    draw_text_shadow(&f, VEC2F(10, 110), 0.7f, WHITE, VEC2F(3, 3), BLACK, "X: %d Y: %d (Chunk: %d)",
                     (int)player.attribs.pos.x / BLOCK_SIZE, 
                     MAX_CHUNK_HEIGHT - (int)(player.attribs.pos.y / BLOCK_SIZE),
                     (int)player.attribs.pos.x / BLOCK_SIZE / CHUNK_SIZE);
    draw_text_shadow(&f, VEC2F(10, 160), 0.7f, WHITE, VEC2F(3, 3), BLACK, "Chunks generated: %d", hm_size(chunks));
}
