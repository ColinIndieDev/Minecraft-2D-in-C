#define CPL_IMPLEMENTATION
#include <cpl/cpl.h>
#define FNL_IMPL
//#include "noise.h"

#include "blocktypes.h"
#include "chunk.h"
#include "player.h"

#define CPRNG_IMPL
#include <cpstd/cprng.h>

player_t player = {.pos = VEC2F(0, 0),
                   .size = VEC2F(0.5f * BLOCK_SIZE, 1.75f * BLOCK_SIZE),
                   .vel = VEC2F(0, 0),
                   .ground = true,
                   .jmp_force = 450.0f,
                   .gravity = 900.0f,
                   .move_speed = 250.0f,
                   .max_fall_speed = 1100.0f};

MAIN_PROG main(void) {
    cprng_rand_seed();
    init_window(800, 800, "Hello CPL", OPENGL_VER_3_3);
    enable_vsync(false);
    font f;
    create_font(&f, "assets/fonts/default.ttf", "default", FILTER_LINEAR);

    fnl_state noise = fnlCreateState();
    noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise.frequency = 0.01f;
    noise.fractal_type = FNL_FRACTAL_FBM;
    noise.octaves = 4;

    // Temporarily, but it actually does not suck, it generates fast + only ~3GB
    // which is lwk wild
    chunk c[MAP_SIZE];
    for (i32 i = 0; i < MAP_SIZE; i++) {
        chunk_gen(&c[i], VEC2F(i, 0), &noise);
        tilemap_check_collidable_tiles(&c[i].tiles,
                                       VEC2F(BLOCK_SIZE, BLOCK_SIZE));
    }

    set_spawn_point(&player, &noise);

    while (!window_should_close()) {
        update();

        handle_controls(&player, c);
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

        display_details(&f);

        end_frame();
    }
    close_window();
}
