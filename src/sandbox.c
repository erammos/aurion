#include <SDL2/SDL.h>
#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_mode_t.h>
#include <world.h>
#include "cglm/affine-pre.h"
#include "cglm/mat4.h"
#include "cglm/vec3.h"
#include "gui.h"

#define WIDTH  800
#define HEIGHT 600

static void
draw_mesh(g_mesh mesh, mat4 parent, vec3 translate, vec3 scale, vec3 rotate, float angle) {
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model, translate);
    glm_scale(model, scale);
    glm_rotate(model, radians(angle), rotate);
    mat4 out;
    glm_mat4_mul(parent, model, out);
    graphics_set_transform(out);
    graphics_draw_mesh(&mesh);
}

static void
draw_mesh_transform(g_mesh mesh, mat4 model) {
    graphics_set_transform(model);
    graphics_draw_mesh(&mesh);
}

typedef struct {
    vec3 pos;
    float yaw;
    float pitch;
} player_t;

#define MAX_CUBES 1

int
main(void) {
    SDL_Init(SDL_INIT_EVERYTHING);
    auto window = SDL_CreateWindow("my test window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_ShowCursor(false);

    graphics_init(window);
    gui_init(window);

    g_shader shader;
    graphics_load_shaders(&shader, "assets/shader.vert", "assets/shader.frag");
    g_shader light_shader;
    graphics_load_shaders(&light_shader, "assets/light.vert", "assets/light.frag");

    auto mesh_obj = graphics_load_obj("assets/test.obj");
    auto mesh_terrain = graphics_create_terrain(100, 100);
    SDL_Event event;
    create_world();
    graphics_use_shader(&light_shader);
    bool running = true;
    unsigned int old_time, current_time;
    Uint32 start_time = SDL_GetTicks();

    current_time = SDL_GetTicks();
    vec2 input_axis = {};
    player_t player = (player_t){.yaw = -90.0f, .pitch = 0, .pos[1] = 5, .pos[2] = 5};
    int mouse_pos[2] = {0};
    graphics_camera_perspective();
    int frame_count = 0;
    char fps[10] = {0};
    while (running) {
        current_time = SDL_GetTicks();
        float delta = (float)(current_time - old_time) / 1000.0f;
        gui_input_begin();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                } else if (event.key.keysym.sym == SDLK_w) {
                    input_axis[1] = 1.0;
                } else if (event.key.keysym.sym == SDLK_a) {
                    input_axis[0] = -1.0;
                } else if (event.key.keysym.sym == SDLK_s) {
                    input_axis[1] = -1.0;
                } else if (event.key.keysym.sym == SDLK_d) {
                    input_axis[0] = 1.0;
                }
            } else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_w) {
                    input_axis[1] = 0.0;
                } else if (event.key.keysym.sym == SDLK_a) {
                    input_axis[0] = 0.0;
                } else if (event.key.keysym.sym == SDLK_s) {
                    input_axis[1] = 0.0;
                } else if (event.key.keysym.sym == SDLK_d) {
                    input_axis[0] = 0.0;
                }
            }
            if (event.type == SDL_MOUSEMOTION) {

                // Update camera view angles
                mouse_pos[0] = event.motion.xrel;
                // Y Coordinates are in screen space so don't get negated
                mouse_pos[1] = event.motion.yrel;
            }
            gui_input(event);
        }

        gui_input_end();
        player.yaw += 0.5f * mouse_pos[0];
        player.pitch -= 0.5f * mouse_pos[1];

        mouse_pos[0] = 0;
        mouse_pos[1] = 0;

        vec3 forward;
        vec3 right;
        glm_vec3_scale(graphics_get_active_camera().front, 100 * delta * input_axis[1], forward);
        glm_vec3_scale(graphics_get_active_camera().right, 100 * delta * input_axis[0], right);
        glm_vec3_add(player.pos, forward, player.pos);
        glm_vec3_add(player.pos, right, player.pos);

        world_update(delta);
        graphics_begin();
        graphics_set_camera(player.pos, (vec3){0.0f, 1.0f, 0.0f}, player.yaw, player.pitch);
        graphics_set_light((vec3){0, 2, 0}, player.pos);

        mat4 model_terrain = GLM_MAT4_IDENTITY_INIT;
        glm_translate(model_terrain, (vec3){-100 / 2, -3, -100 / 2});
        draw_mesh_transform(mesh_terrain, model_terrain);
        draw_mesh_transform(mesh_obj, model_terrain);
        gui_begin();

        gui_draw_text(graphics_get_width() / 2, graphics_get_height() / 2, "+");
        gui_draw_text(10, 10, fps);
        Uint32 current_time = SDL_GetTicks();
        if (current_time - start_time >= 1000) {
            sprintf(fps, "%d", frame_count);
            frame_count = 0;
            start_time = current_time;
        }
        gui_end();
        graphics_end();
        frame_count++;

        old_time = current_time;
    }

    graphics_destroy();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
