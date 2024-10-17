#include <SDL2/SDL.h>
#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <world.h>
#include "camera.h"
#include "cglm/affine-pre.h"
#include "cglm/affine.h"
#include "cglm/call/affine.h"
#include "cglm/io.h"
#include "cglm/mat4.h"
#include "cglm/util.h"
#include "cglm/vec3.h"
#include "gui.h"
#include "input.h"

#define WIDTH  800
#define HEIGHT 600


typedef struct {
    vec3 pos;
    float yaw;
    float pitch;
} player_t;

#define MAX_CUBES 1

char front_log[100] = {0};
char rot_log[100] = {0};

void
player_move(g_entity e, vec3 mouse_pos, vec3 input_axis, float speed, float sensitivity, float dt) {

    mat4* matrix;
    world_get_local_transform(e, &matrix);

    g_rotation* g_rotation = world_get_rotation(e);
    vec2 dir;
    float scale = speed * dt * input_axis[1];
    dir[0] = sin(glm_rad(g_rotation->rotation[1]));
    dir[1] = cos(glm_rad(g_rotation->rotation[1]));
    sprintf(front_log, "%f %f", dir[0], dir[1]);
    sprintf(rot_log, "%f %f %f", g_rotation->rotation[0], g_rotation->rotation[1], g_rotation->rotation[2]);
    g_position* g_pos = world_get_position(e);
    g_scale* g_scale = world_get_scale(e);

    g_pos->position[0] += scale * dir[0];
    g_pos->position[2] += scale * dir[1];
    g_rotation->rotation[1] -= sensitivity * mouse_pos[0];
    g_rotation->rotation[0] += sensitivity * mouse_pos[1];
    g_rotation->rotation[2] = 0;

    world_reset_entity(e);
    world_translate_entity(e, g_pos->position);
    world_rotate_entity(e, g_rotation->rotation[1], (vec3){0, 1, 0});
    world_rotate_entity(e, g_rotation->rotation[0], (vec3){1, 0, 0});
}

int
main(void) {
    SDL_Init(SDL_INIT_EVERYTHING);
    auto window = SDL_CreateWindow("my test window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_ShowCursor(false);
    SDL_SetRelativeMouseMode(true);

    graphics_init(window);
    gui_init(window);

    g_shader light_shader;
    graphics_load_shaders(&light_shader, "assets/light.vert", "assets/light.frag");
    g_shader orb_shader;
    graphics_load_shaders(&orb_shader, "assets/emissive.vert", "assets/emissive.frag");

    create_world();
    auto mesh_terrain = graphics_create_terrain(100, 100);
    auto terrain = world_create_entity("terrain", (vec3){0, 0, 0}, (vec3){1, 1, 1}, (vec3){0, 0, 0}, world);
    world_add_mesh(terrain, &mesh_terrain);

    //    auto cube = world_create_entity("cube", (vec3){50, 0.5f, 50}, (vec3){1, 1, 2},  (vec3){0, 0, 0}, terrain.entity);
    g_entity player = world_create_entity("player", (vec3){50, 0.5f, 50}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0, 0, 0}, world);
    auto e_camera = world_create_entity("camera", (vec3){0, 10, -10}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){30, -180, 0},
                                        player.entity);

    auto mesh_obj = graphics_load_obj("assets/test.obj");
    world_add_mesh(player, &mesh_obj);
    bool running = true;
    unsigned int old_time, current_time;
    Uint32 start_time = SDL_GetTicks();

    current_time = SDL_GetTicks();
    vec3 input_axis = {};
    vec3 mouse_pos = {0};
    int frame_count = 0;
    char fps[10] = {0};
    g_camera camera = camera_create(graphics_get_width(), graphics_get_height());
    camera_update(&camera, (vec3){50, 1.0f, 50 - 5}, (vec3){0, 1, 0}, -180, 0);

    while (running) {
        current_time = SDL_GetTicks();
        float delta = (float)(current_time - old_time) / 1000.0f;
        running = input_update(input_axis, mouse_pos);

        player_move(player, mouse_pos, input_axis, 10, 0.5f, delta);
        world_update(delta);

        mat4* model;
        world_get_world_transform(e_camera, &model);
        glm_mat4_inv(*model, camera.view);



        graphics_begin();
        graphics_use_shader(&light_shader);
        graphics_use_camera(&camera);
        world_draw();

        gui_begin();
        gui_draw_text(graphics_get_width() / 2, graphics_get_height() / 2, "+");
        gui_draw_text(10, 10, fps);
        gui_draw_text(10, 50, front_log);
        gui_draw_text(10, 90, rot_log);
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
