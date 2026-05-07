#include <cpstd/cpbase.h>
#include <cpstd/cpmath.h>
#include <cpstd/cpmemory.h>
#include <cpstd/cprng.h>
#define CPL_IMPLEMENTATION
#include <cpl/cpl.h>

#define FNL_IMPL
#include "../external/fastnoiselite/FastNoiseLite.h"

#define CHUNK_SIZE 16
#define MAX_CHUNK_HEIGHT 256

#define MAP_SIZE 10000

#define MIN_TERRAIN_HEIGHT 40
#define SEA_LEVEL 60
#define MAX_FIELD_HEIGHT 80
#define MAX_HILL_HEIGHT 100

#define BLOCK_SIZE 75

#define BLOCK_GRASS_BLOCK VEC2F(1, 0)
#define BLOCK_DIRT VEC2F(0, 0)
#define BLOCK_STONE VEC2F(2, 0)
#define BLOCK_SAND VEC2F(0, 1)
#define BLOCK_BEDROCK VEC2F(1, 1)
#define BLOCK_WATER VEC2F(2, 1)
#define BLOCK_FLOWER_ROSE VEC2F(0, 2)
#define BLOCK_SUGAR_CANE VEC2F(1, 2)
#define BLOCK_OAK_LOG VEC2F(3, 0)
#define BLOCK_OAK_LEAVES VEC2F(3, 1)

typedef struct {
    vec2f pos;
    tilemap tiles;
} chunk;
void gen_tree(chunk *c, vec2f pos, u32 x);
void gen_trees(chunk *c, vec2f pos, fnl_state *terrain);
void gen_foliage(chunk *c, vec2f pos, fnl_state *terrain);
void chunk_gen(chunk *c, vec2f pos, fnl_state *terrain);
void chunk_draw(chunk *c) { cpl_tilemap_draw(&c->tiles); }

void handle_cam();
void set_spawn_point(fnl_state *terrain);

f32 cam_speed = 1000.0f;

MAIN_PROG main(void) {
    cprng_rand_seed();
    cpl_init_window(800, 800, "Hello CPL", OPENGL_VER_3_3);
    cpl_enable_vsync(false);
    font f;
    cpl_create_font(&f, "assets/fonts/default.ttf", "default", FILTER_LINEAR);

    fnl_state noise = fnlCreateState();
    noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise.frequency = 0.01f;
    noise.fractal_type = FNL_FRACTAL_FBM;
    noise.octaves = 4;

    // Temporarily, but it actually does not suck, it generates fast + only ~3GB
    // which is lwk wild
    chunk c[MAP_SIZE];
    for (i32 i = -MAP_SIZE / 2; i < (MAP_SIZE / 2) - 1; i++) {
        chunk_gen(&c[i + (MAP_SIZE / 2)], VEC2F(i, 0), &noise);
    }

    set_spawn_point(&noise);

    while (!cpl_window_should_close()) {
        cpl_update();

        handle_cam();

        cpl_clear_background(LIGHT_BLUE);

        cpl_begin_draw(TEXTURE_2D_UNLIT, true);

        for (u32 i = 0; i < MAP_SIZE; i++) {
            // Do not necessarily need rect collider since chunk
            // pos on y axis does not change
            rect_collider screen = (rect_collider){
                .pos = cpl_get_cam_2D()->pos,
                .size = VEC2F(
                    cpl_get_screen_width() * (1 / cpl_get_cam_2D()->zoom),
                    cpl_get_screen_height() * (1 / cpl_get_cam_2D()->zoom))};
            rect_collider chunk = (rect_collider){
                .pos = VEC2F(c[i].pos.x * CHUNK_SIZE * BLOCK_SIZE, 0),
                .size = VEC2F(CHUNK_SIZE * BLOCK_SIZE,
                              MAX_CHUNK_HEIGHT * BLOCK_SIZE)};
            if (cpl_check_collision_rects(screen, chunk)) {
                chunk_draw(&c[i]);
            }
        }

        cpl_display_details(&f);

        cpl_end_frame();
    }
    cpl_close_window();
}

void chunk_gen(chunk *c, vec2f pos, fnl_state *terrain) {
    cpl_create_tilemap(&c->tiles, VEC2F_INIT(16));
    cpl_tilemap_load_texture(&c->tiles, "assets/images/block_map.png",
                             FILTER_NEAREST);
    c->pos = pos;
    cpl_tilemap_begin_editing(&c->tiles);
    for (u32 x = 0; x < CHUNK_SIZE; x++) {
        f32 noise =
            (fnlGetNoise2D(terrain, (f32)((u32)((pos.x + (MAP_SIZE / 2)) * CHUNK_SIZE) + x), 0) +
             1.0f) *
            0.5f;
        u32 height = MIN_TERRAIN_HEIGHT +
                     (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));
        for (u32 y = 0; y < height; y++) {
            vec2f uv;
            if (y == height - 1) {
                if (height <= SEA_LEVEL) {
                    uv = BLOCK_SAND;
                    for (u32 w = y; w < SEA_LEVEL; w++) {
                        cpl_tilemap_add_tile(
                            &c->tiles,
                            VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                                  (MAX_CHUNK_HEIGHT - w) * BLOCK_SIZE),
                            VEC2F_INIT(BLOCK_SIZE), BLOCK_WATER);
                    }
                } else {
                    uv = BLOCK_GRASS_BLOCK;
                }
            } else if (y > height - 7) {
                if (height <= SEA_LEVEL) {
                    uv = BLOCK_SAND;
                } else {
                    uv = BLOCK_DIRT;
                }
            } else if (y == 0) {
                uv = BLOCK_BEDROCK;
            } else {
                uv = BLOCK_STONE;
            }

            cpl_tilemap_add_tile(&c->tiles,
                                 VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                                       (MAX_CHUNK_HEIGHT - y) * BLOCK_SIZE),
                                 VEC2F_INIT(BLOCK_SIZE), uv);
        }
    }

    gen_foliage(c, VEC2F(pos.x, pos.y), terrain);
    gen_trees(c, VEC2F(pos.x, pos.y), terrain);
}

void gen_foliage(chunk *c, vec2f pos, fnl_state *terrain) {
    for (u32 x = 0; x < CHUNK_SIZE; x++) {
        f32 noise =
            (fnlGetNoise2D(terrain, (f32)((u32)((pos.x + (MAP_SIZE / 2)) * CHUNK_SIZE) + x), 0) +
             1.0f) *
            0.5f;
        u32 height = MIN_TERRAIN_HEIGHT +
                     (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));

        if (cprng_rand() % 5 == 0) {
            if (height > SEA_LEVEL) {
                cpl_tilemap_add_tile(
                    &c->tiles,
                    VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                          (MAX_CHUNK_HEIGHT - height) * BLOCK_SIZE),
                    VEC2F_INIT(BLOCK_SIZE), BLOCK_FLOWER_ROSE);
            } else if (height == SEA_LEVEL) {
                for (u32 y = 0; y < cprng_rand_range(1, 5); y++) {
                    cpl_tilemap_add_tile(
                        &c->tiles,
                        VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                              (MAX_CHUNK_HEIGHT - height - y) * BLOCK_SIZE),
                        VEC2F_INIT(BLOCK_SIZE), BLOCK_SUGAR_CANE);
                }
            }
        }
    }
}

void gen_tree(chunk *c, vec2f pos, u32 x) {
    u32 tree_height = cprng_rand_range(2, 7);
    for (u32 ly = tree_height - 2; ly < tree_height - 2 + 4; ly++) {
        if (ly == tree_height - 2 + 4 - 1) {
            for (i32 lx = -1; lx < 2; lx++) {
                cpl_tilemap_add_tile(
                    &c->tiles,
                    VEC2F(((pos.x * CHUNK_SIZE) + x + lx) * BLOCK_SIZE,
                          (MAX_CHUNK_HEIGHT - pos.y - ly) * BLOCK_SIZE),
                    VEC2F_INIT(BLOCK_SIZE), BLOCK_OAK_LEAVES);
            }
        } else {
            for (i32 lx = -2; lx < 3; lx++) {
                cpl_tilemap_add_tile(
                    &c->tiles,
                    VEC2F(((pos.x * CHUNK_SIZE) + x + lx) * BLOCK_SIZE,
                          (MAX_CHUNK_HEIGHT - pos.y - ly) * BLOCK_SIZE),
                    VEC2F_INIT(BLOCK_SIZE), BLOCK_OAK_LEAVES);
            }
        }
    }
    for (u32 y = 0; y < tree_height; y++) {
        cpl_tilemap_add_tile(&c->tiles,
                             VEC2F(((pos.x * CHUNK_SIZE) + x) * BLOCK_SIZE,
                                   (MAX_CHUNK_HEIGHT - pos.y - y) * BLOCK_SIZE),
                             VEC2F_INIT(BLOCK_SIZE), BLOCK_OAK_LOG);
    }
}

void gen_trees(chunk *c, vec2f pos, fnl_state *terrain) {
    for (u32 x = 0; x < CHUNK_SIZE; x++) {
        f32 noise =
            (fnlGetNoise2D(terrain, (f32)((u32)((pos.x + (MAP_SIZE / 2)) * CHUNK_SIZE) + x), 0) +
             1.0f) *
            0.5f;
        u32 height = MIN_TERRAIN_HEIGHT +
                     (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));

        if (cprng_rand() % 5 == 0 && height > SEA_LEVEL) {
            gen_tree(c, VEC2F(pos.x, height), x);
        }
    }
}

void handle_cam() {
    if (cpl_is_key_down(KEY_W)) {
        cpl_get_cam_2D()->pos.y -= cam_speed * cpl_get_dt();
    }
    if (cpl_is_key_down(KEY_S)) {
        cpl_get_cam_2D()->pos.y += cam_speed * cpl_get_dt();
    }
    if (cpl_is_key_down(KEY_A)) {
        cpl_get_cam_2D()->pos.x -= cam_speed * cpl_get_dt();
    }
    if (cpl_is_key_down(KEY_D)) {
        cpl_get_cam_2D()->pos.x += cam_speed * cpl_get_dt();
    }
    if (cpl_is_key_down(KEY_ESCAPE)) {
        cpl_destroy_window();
    }

    if (cpl_is_key_down(KEY_G)) {
        cam_speed += cam_speed * 10 * cpl_get_dt();
    }
    if (cpl_is_key_down(KEY_B)) {
        cam_speed -= cam_speed * 10 * cpl_get_dt();
    }
    cam_speed = CPM_CLAMP(cam_speed, 10, 1000000);

    if (cpl_is_key_down(KEY_H)) {
        cpl_get_cam_2D()->zoom += 2 * cpl_get_dt();
    }
    if (cpl_is_key_down(KEY_N)) {
        cpl_get_cam_2D()->zoom -= 2 * cpl_get_dt();
    }
    cpl_get_cam_2D()->zoom = CPM_CLAMP(cpl_get_cam_2D()->zoom, 0.01f, 10);
}

void set_spawn_point(fnl_state *terrain) {
    f32 noise =
        (fnlGetNoise2D(terrain, (f32)((u32)(0 * CHUNK_SIZE)), 0) + 1.0f) * 0.5f;
    u32 height =
        MIN_TERRAIN_HEIGHT + (noise * (MAX_FIELD_HEIGHT - MIN_TERRAIN_HEIGHT));
    u32 height_off = 6;
    cpl_get_cam_2D()->pos.y =
        (f32)(MAX_CHUNK_HEIGHT - height - height_off) * BLOCK_SIZE;
}
