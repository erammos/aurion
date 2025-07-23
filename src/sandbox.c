#include <SDL2/SDL.h>
#include <ecs.h>
#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>

#include "assets.h"
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
// // Add this helper function somewhere in your code.
// // It calculates the height of a point (p) within a triangle (a, b, c)
// float barycentric_height(vec3 p, vec3 a, vec3 b, vec3 c) {
//     vec3 v0 = {b[0] - a[0], 0, b[2] - a[2]};
//     vec3 v1 = {c[0] - a[0], 0, c[2] - a[2]};
//     vec3 v2 = {p[0] - a[0], 0, p[2] - a[2]};
//
//     float d00 = glm_vec3_dot(v0, v0);
//     float d01 = glm_vec3_dot(v0, v1);
//     float d11 = glm_vec3_dot(v1, v1);
//     float d20 = glm_vec3_dot(v2, v0);
//     float d21 = glm_vec3_dot(v2, v1);
//
//     float denom = d00 * d11 - d01 * d01;
//     if (fabs(denom) < 1e-5) { // Avoid division by zero
//         return a[1];
//     }
//
//     float v = (d11 * d20 - d01 * d21) / denom;
//     float w = (d00 * d21 - d01 * d20) / denom;
//     float u = 1.0f - v - w;
//
//     // Use the weights u, v, w to interpolate the Y-values (height)
//     return u * a[1] + v * b[1] + w * c[1];
// }

// Gets the height on the terrain mesh at world coordinates (x, z)
float get_height_on_terrain(float x, float z, c_mesh terrain) {
    int terrain_width = 100; // Must match the width used in graphics_create_terrain
    int terrain_depth = 100; // Must match the depth used in graphics_create_terrain

    // Find the integer grid coordinates by truncating the floats
    int grid_x = (int)x;
    int grid_z = (int)z;

    // --- Boundary Check ---
    // Ensure the player is within the valid bounds of the terrain grid.
    if (grid_x < 0 || grid_x >= terrain_width - 1 || grid_z < 0 || grid_z >= terrain_depth - 1) {
        return 0.0f; // Return a default height if outside the terrain
    }

    // --- Fractional Coordinates ---
    // Calculate how far the player is across the grid cell (from 0.0 to 1.0)
    float frac_x = x - grid_x;
    float frac_z = z - grid_z;

    // --- Get the Four Corner Vertices ---
    // Get the vertices of the quad the player is standing on.
    g_vertex v00 = terrain.vertices[grid_z * terrain_width + grid_x];           // Top-Left
    g_vertex v10 = terrain.vertices[grid_z * terrain_width + (grid_x + 1)];     // Top-Right
    g_vertex v01 = terrain.vertices[(grid_z + 1) * terrain_width + grid_x];     // Bottom-Left
    g_vertex v11 = terrain.vertices[(grid_z + 1) * terrain_width + (grid_x + 1)]; // Bottom-Right

    // --- Interpolation ---
    // 1. Interpolate in the X direction for both the top and bottom edges of the quad.
    //    We use glm_lerp to blend between the two heights.
    float height_top = glm_lerp(v00.position[1], v10.position[1], frac_x);
    float height_bottom = glm_lerp(v01.position[1], v11.position[1], frac_x);

    // 2. Interpolate in the Z direction between the two previously calculated heights.
    float final_height = glm_lerp(height_top, height_bottom, frac_z);

    return final_height;
}
void
player_move(g_entity e, vec3 mouse_pos, vec3 input_axis, float speed, float sensitivity, float dt,
            c_mesh mesh_terrain) {

    c_rotation* g_rotation = ecs_get_rotation(e);
    vec2 dir;
    float scale_v = speed * dt * input_axis[1];
    float scale_h = speed * dt * -input_axis[0];
    dir[0] = sin(glm_rad(g_rotation->rotation[1]));
    dir[1] = cos(glm_rad(g_rotation->rotation[1]));
    sprintf(front_log, "%f %f", input_axis[0], input_axis[1]);
    sprintf(rot_log, "%f %f %f", g_rotation->rotation[0], g_rotation->rotation[1], g_rotation->rotation[2]);
    c_position* g_pos = ecs_get_position(e);
    c_scale* g_scale = ecs_get_scale(e);

    g_pos->position[0] += scale_v * dir[0];
    g_pos->position[2] += scale_v * dir[1];
    g_pos->position[0] += scale_h * dir[1];
    g_pos->position[2] -= scale_h * dir[0];
    g_pos->position[1] = get_height_on_terrain(g_pos->position[0], g_pos->position[2], mesh_terrain);

    g_rotation->rotation[1] -= sensitivity * mouse_pos[0];
    g_rotation->rotation[0] += sensitivity * mouse_pos[1];
    g_rotation->rotation[2] = 0;

    ecs_reset_entity(e);
    ecs_translate_entity(e, g_pos->position);
    ecs_rotate_entity(e, g_rotation->rotation[1], (vec3){0, 1, 0});
    ecs_rotate_entity(e, g_rotation->rotation[0], (vec3){1, 0, 0});
    ecs_scale_entity(e, g_scale->scale);

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

     g_pbr_shader = graphics_load_shaders( "assets/light.vert", "assets/light.frag");
    g_cel_shader = graphics_load_shaders("assets/emissive.vert", "assets/emissive.frag");

    auto mesh_terrain = graphics_create_terrain(100, 100);
    auto terrain = ecs_create_entity("terrain", (vec3){0, 0, 0}, (vec3){1, 1, 1}, (vec3){0, 0, 0}, world);
    auto terrain_texture = graphics_load_texture("assets/marble2.jpg");
    ecs_add_mesh(terrain, &mesh_terrain);
    ecs_add_texture(terrain,&terrain_texture);
    ecs_use_pbr_shader(terrain);

    //    auto cube = world_create_entity("cube", (vec3){50, 0.5f, 50}, (vec3){1, 1, 2},  (vec3){0, 0, 0}, terrain.entity);
    g_entity player = ecs_create_entity("player", (vec3){0, 0, 0}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0, 0, 0}, world);
    auto e_camera = ecs_create_entity("camera", (vec3){0, 10, -20}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){30, -180, 0},
                                        world);

  //  auto mesh_obj = graphics_load_model("assets/sphere.obj");
    auto mesh_obj = graphics_load_model("assets/skeleton.gltf", nullptr);
    ecs_add_mesh(player, &mesh_obj);
    ecs_add_texture(player, &terrain_texture);
    ecs_use_pbr_shader(player);

    auto orb_obj = graphics_load_model("assets/sphere.gltf", nullptr);
    g_entity light_origin = ecs_create_entity("origin", (vec3){0.0f, 0.0f, 0}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0, 0, 0}, player.entity);
    g_entity light = ecs_create_entity("light", (vec3){0, 3.0f, 10}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0, 0, 0}, light_origin.entity);
    ecs_add_mesh(light, &orb_obj);
    ecs_use_emissive_shader(light,(c_emission){.centerPosition = {0.0,0.0,0.0},
               .orbColor = {1.0,0.8,1.0},
               .intensity = 1000.0f,
               .radius = 1.01f});

    // for (int i = 0 ; i < 10; i++) {
    //
    //     for (int j = 0 ; j < 10; j++) {
    //         char name[20] = {};
    //         snprintf(name, 20, "cube%i", i*100 + j);
    //         g_entity cube = ecs_create_entity(name, (vec3){ i, 3, j}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0, 0, 0}, world);
    //         ecs_add_mesh(cube, &mesh_obj);
    //         ecs_add_texture(cube, &player_texture);
    //         ecs_use_pbr_shader(cube);
    //
    //
    //
    //     }
    // }


    unsigned int prev_time = SDL_GetTicks();

    vec3 input_axis = {};
    vec3 mouse_pos = {0};
    g_camera camera = camera_create(graphics_get_width(), graphics_get_height());
    camera_update(&camera, (vec3){50, 1.0f, 50 - 5}, (vec3){0, 1, 0}, -180, 0);
    bool running = true;
    float offset = 0.0f;
    while (running) {
        Uint32 current_time = SDL_GetTicks();
        float delta = (float)(current_time - prev_time) / 1000;
        prev_time = current_time;
        running = input_update(input_axis, mouse_pos);

        player_move(player, mouse_pos, input_axis, 10, 0.5f, delta, mesh_terrain);
       auto players_pos = ecs_get_world_position(player);
        glm_ivec3_add(players_pos.position,(vec3) {0,10,-10},camera.pos);
        camera_look_at(&camera, players_pos.position,(vec3){0,1,0});

        ecs_reset_entity(light_origin);
        ecs_rotate_entity(light_origin,offset,(vec3) {0,1,0});
        offset+=delta * 100.0f;
        ecs_run_update_system(delta);

       //  mat4* model = ecs_get_world_transform(e_camera);
       // // ecs_scale_entity(e_camera,)
       //  glm_mat4_inv(*model, camera.view);


        graphics_begin();
        auto pos = ecs_get_world_position(light);
        auto player_pos = ecs_get_world_position(player);
        ecs_set_light(pos.position,player_pos.position,(vec3){1.0f,1.0f,1.0f});
        ecs_set_camera(&camera);
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
