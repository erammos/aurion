#include <SDL2/SDL.h>
#include <ecs.h>
#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
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
    ecs_get_local_transform(e, &matrix);

    g_rotation* g_rotation = ecs_get_rotation(e);
    vec2 dir;
    float scale = speed * dt * input_axis[1];
    dir[0] = sin(glm_rad(g_rotation->rotation[1]));
    dir[1] = cos(glm_rad(g_rotation->rotation[1]));
    sprintf(front_log, "%f %f", dir[0], dir[1]);
    sprintf(rot_log, "%f %f %f", g_rotation->rotation[0], g_rotation->rotation[1], g_rotation->rotation[2]);
    g_position* g_pos = ecs_get_position(e);
    g_scale* g_scale = ecs_get_scale(e);

    g_pos->position[0] += scale * dir[0];
    g_pos->position[2] += scale * dir[1];
    g_rotation->rotation[1] -= sensitivity * mouse_pos[0];
    g_rotation->rotation[0] += sensitivity * mouse_pos[1];
    g_rotation->rotation[2] = 0;

    ecs_reset_entity(e);
    ecs_translate_entity(e, g_pos->position);
    ecs_rotate_entity(e, g_rotation->rotation[1], (vec3){0, 1, 0});
    ecs_rotate_entity(e, g_rotation->rotation[0], (vec3){1, 0, 0});
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
    ecs_init_systems();

    g_shader light_shader = graphics_load_shaders( "assets/light.vert", "assets/light.frag");
    g_shader orb_shader = graphics_load_shaders("assets/emissive.vert", "assets/emissive.frag");

    auto mesh_terrain = graphics_create_terrain(100, 100);
    auto terrain = ecs_create_entity("terrain", (vec3){0, 0, 0}, (vec3){1, 1, 1}, (vec3){0, 0, 0}, world);
    ecs_add_mesh(terrain, &mesh_terrain);

    //    auto cube = world_create_entity("cube", (vec3){50, 0.5f, 50}, (vec3){1, 1, 2},  (vec3){0, 0, 0}, terrain.entity);
    g_entity player = ecs_create_entity("player", (vec3){50, 0.5f, 50}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0, 0, 0}, world);
    auto e_camera = ecs_create_entity("camera", (vec3){0, 10, -10}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){30, -180, 0},
                                        player.entity);

    auto mesh_obj = graphics_load_obj("assets/sphere.obj");
    ecs_add_mesh(player, &mesh_obj);
    unsigned int prev_time = SDL_GetTicks();

    vec3 input_axis = {};
    vec3 mouse_pos = {0};
    g_camera camera = camera_create(graphics_get_width(), graphics_get_height());
    camera_update(&camera, (vec3){50, 1.0f, 50 - 5}, (vec3){0, 1, 0}, -180, 0);
    bool running = true;
    while (running) {
        Uint32 current_time = SDL_GetTicks();
        float delta = (float)(current_time - prev_time) / 1000;
        prev_time = current_time;
        running = input_update(input_axis, mouse_pos);

        player_move(player, mouse_pos, input_axis, 10, 0.5f, delta);
        ecs_run_update_system(delta);

        mat4* model = ecs_get_world_transform(e_camera);
        glm_mat4_inv(*model, camera.view);



        graphics_begin();
        graphics_use_shader(&light_shader);
        graphics_use_camera(&camera);
        graphics_set_light((vec3){-1.0f,10.0f,0}, camera.pos,0.8f,180.0f);
        ecs_run_render_system();

        gui_begin();
        gui_draw_text(graphics_get_width() / 2, graphics_get_height() / 2, "+");
        gui_draw_text(10, 50, front_log);
        gui_draw_text(10, 90, rot_log);
        gui_draw_fps();
        gui_end();
        graphics_end();
    }

    graphics_destroy();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
