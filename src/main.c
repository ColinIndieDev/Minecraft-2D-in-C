#include <stdio.h>
#define CPL_IMPLEMENTATION
#include <cpl/cpl.h>
#define FNL_IMPL
// #include "noise.h"

#include "blocktypes.h"
#include "chunk.h"
#include "player.h"

#define CPRNG_IMPL
#include <cpstd/cprng.h>

vec2f block_types_uv[BLOCK_TYPES] = {
    VEC2F(1, 0), // Grass Block
    VEC2F(0, 0), // Dirt
    VEC2F(2, 0), // Stone
    VEC2F(0, 1), // Sand
    VEC2F(1, 1), // Bedrock
    VEC2F(2, 1), // Water
    VEC2F(0, 2), // Rose
    VEC2F(1, 2), // Sugar Cane
    VEC2F(3, 0), // Oak Log
    VEC2F(3, 1)  // Oak Leaves
};

f32 block_types_mining_dt[BLOCK_TYPES] = {
    1.0f,  // Grass Block
    1.0f,  // Dirt
    10.0f, // Stone
    0.6f,  // Sand
    -1.0f, // Bedrock
    -1.0f, // Water
    0.1f,  // Rose
    0.3f,  // Sugar Cane
    5.0f,  // Oak Log
    0.3f   // Oak Leaves
};

player_t player = {.pos = VEC2F(0, 0),
                   .size = VEC2F(0.5f * BLOCK_SIZE, 1.75f * BLOCK_SIZE),
                   .vel = VEC2F(0, 0),
                   .ground = true,
                   .jmp_force = 450.0f,
                   .gravity = 900.0f,
                   .move_speed = 250.0f,
                   .max_fall_speed = 1100.0f,
                   .block_mining = VEC2F(-1, -1),
                   .block_mining_dt = 0.0f,
                   .block_mining_timer = 0.0f};

font f;

map_noise_t map_noise;

void draw_ui();

int main(void) {
    cprng_rand_seed();
    init_window(800, 800, "Hello CPL", OPENGL_VER_3_3);
    enable_vsync(false);
    create_font(&f, "assets/fonts/default.ttf", "default", FILTER_LINEAR);

    gen_seed(&map_noise);

    // Temporarily, but it actually does not suck, it generates fast + only ~3GB
    // which is lwk wild
    chunk c[MAP_SIZE];
    for (i32 i = 0; i < MAP_SIZE; i++) {
        chunk_gen(&c[i], VEC2F(i, 0), &map_noise, block_types_uv);
        tilemap_check_collidable_tiles(&c[i].tiles,
                                       VEC2F(BLOCK_SIZE, BLOCK_SIZE));
    }

    set_spawn_point(&player, &map_noise.terrain);

    while (!window_should_close()) {
        update();

        handle_controls(&player, c, block_types_uv, block_types_mining_dt);
        move_and_collide(&player, c);

        clear_background(LIGHT_BLUE);

        begin_draw(TEXTURE_2D_UNLIT, true);

        i32 idx = (i32)player.pos.x / (CHUNK_SIZE * BLOCK_SIZE);

        for (i32 i = -1; i < 2; i++) {
            if (idx < 0 || idx >= MAP_SIZE) {
                break;
            }
            if (idx == 0 && i == -1) {
                continue;
            }
            if (idx == MAP_SIZE - 1 && i == 1) {
                continue;
            }
            // Do not necessarily need rect collider since chunk
            // pos on y axis does not change
            rect_collider screen = (rect_collider){
                .pos = get_cam_2D()->pos,
                .size = VEC2F(get_screen_width() * (1 / get_cam_2D()->zoom),
                              get_screen_height() * (1 / get_cam_2D()->zoom))};
            rect_collider chunk = (rect_collider){
                .pos = VEC2F(c[idx + i].pos.x * CHUNK_SIZE * BLOCK_SIZE, 0),
                .size = VEC2F(CHUNK_SIZE * BLOCK_SIZE,
                              MAX_CHUNK_HEIGHT * BLOCK_SIZE)};
            if (check_collision_rects(screen, chunk)) {
                chunk_draw(&c[idx + i]);
            }
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

        for (i32 i = -1; i < 2; i++) {
            if (idx < 0 || idx >= MAP_SIZE) {
                break;
            }
            if (idx == 0 && i == -1) {
                continue;
            }
            if (idx == MAP_SIZE - 1 && i == 1) {
                continue;
            }
            // Do not necessarily need rect collider since chunk
            // pos on y axis does not change
            rect_collider screen = (rect_collider){
                .pos = get_cam_2D()->pos,
                .size = VEC2F(get_screen_width() * (1 / get_cam_2D()->zoom),
                              get_screen_height() * (1 / get_cam_2D()->zoom))};
            rect_collider chunk = (rect_collider){
                .pos = VEC2F(c[idx + i].pos.x * CHUNK_SIZE * BLOCK_SIZE, 0),
                .size = VEC2F(CHUNK_SIZE * BLOCK_SIZE,
                              MAX_CHUNK_HEIGHT * BLOCK_SIZE)};
            if (check_collision_rects(screen, chunk)) {
                chunk_draw_passable(&c[idx + i]);
            }
        }

        begin_draw(TEXT, false);

        draw_ui();
        // display_details(&f);

        end_frame();
    }
    close_window();
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
