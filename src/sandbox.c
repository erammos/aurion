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
void player_move(g_entity e, vec3 mouse_pos, vec3 input_axis, float speed, float sensitivity, float dt, c_mesh mesh_terrain) {
    // Get the player's core components
    c_position* g_pos = ecs_get_position(e);
    // Use the existing c_rotation component to store yaw and pitch
    c_rotation* g_rotation = ecs_get_rotation(e);

    // 1. UPDATE ANGLES based on mouse input
    // g_rotation->rotation[1] will be our Yaw
    // g_rotation->rotation[0] will be our Pitch
    g_rotation->rotation[1] += mouse_pos[0] * sensitivity;
    g_rotation->rotation[0] -= mouse_pos[1] * sensitivity;

    // 2. CLAMP PITCH to prevent looking too far up or down
    if (g_rotation->rotation[0] > 89.0f) {
        g_rotation->rotation[0] = 89.0f;
    }
    if (g_rotation->rotation[0] < -89.0f) {
        g_rotation->rotation[0] = -89.0f;
    }

    // 3. CALCULATE DIRECTION VECTORS from the new angles
    vec3 front;
    front[0] = cos(glm_rad(g_rotation->rotation[1])) * cos(glm_rad(g_rotation->rotation[0]));
    front[1] = sin(glm_rad(g_rotation->rotation[0]));
    front[2] = sin(glm_rad(g_rotation->rotation[1])) * cos(glm_rad(g_rotation->rotation[0]));
    glm_vec3_normalize(front);

    // Also re-calculate the Right and Up vectors
    vec3 right, up;
    glm_vec3_cross(front, (vec3){0.0f, 1.0f, 0.0f}, right);
    glm_vec3_normalize(right);
    glm_vec3_cross(right, front, up);
    glm_vec3_normalize(up);

    // 4. APPLY MOVEMENT based on input axis and direction vectors
    vec3 move_horizontal, move_vertical;
    glm_vec3_scale(front, speed * dt * input_axis[1], move_vertical);   // W/S movement
    glm_vec3_scale(right, speed * dt * input_axis[0], move_horizontal);  // A/D movement

    glm_vec3_add(g_pos->position, move_vertical, g_pos->position);
    glm_vec3_add(g_pos->position, move_horizontal, g_pos->position);

    // 5. UPDATE PLAYER HEIGHT based on terrain
    g_pos->position[1] = get_height_on_terrain(g_pos->position[0], g_pos->position[2], mesh_terrain) + 2.0f; // Eye level adjustment

    // 6. UPDATE ENTITY TRANSFORM for rendering
    // We create a "look at" matrix from the vectors and then apply the position
    mat4 look_at_matrix;
    vec3 center;
    glm_vec3_add(g_pos->position, front, center);
    glm_lookat(g_pos->position, center, up, look_at_matrix);

    // The entity's final transform is the inverse of the look_at matrix
    mat4* world_transform;
    ecs_get_local_transform(e,&world_transform);
    glm_mat4_inv(look_at_matrix, *world_transform);
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
    g_entity player = ecs_create_entity("player", (vec3){0, 100, 0}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0, 0, 0}, world);


    // g_entity camera_rig = ecs_create_entity("camera_rig", (vec3){0, 0.0f, 0}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0, 0, 0},world);
    // g_entity camera_pivot = ecs_create_entity("camera_pivot", (vec3){0, 5, 0}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0, 0, 0}, camera_rig.entity);
    // g_entity camera_slot = ecs_create_entity("camera_slot", (vec3){0, 5, -20}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){30, -180, 0}, camera_pivot.entity);
    // g_entity e_camera = ecs_create_entity("camera", (vec3){0, 0, 0}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0, 0, 0}, camera_slot.entity);

    ecs_add_camera(player,1.777f);
    ecs_use_pbr_shader(player);

    auto orb_obj = graphics_load_model("assets/sphere.gltf", nullptr);
    g_entity light_origin = ecs_create_entity("origin", (vec3){50.0f, 0.0f, 50}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0, 0, 0}, world);
    g_entity light = ecs_create_entity("light", (vec3){0, 0.0f, 10.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0, 0, 0}, light_origin.entity);
    ecs_add_mesh(light, &orb_obj);
    ecs_use_emissive_shader(light,(c_emission){.centerPosition = {0.0,0.0,0.0},
               .orbColor = {1.0,0.9,1.0},
               .intensity = 1.025f,
               .radius = 5.0f});

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
    bool running = true;
    float offset = 0.0f;
    while (running) {
        Uint32 current_time = SDL_GetTicks();
        float delta = (float)(current_time - prev_time) / 1000;
        prev_time = current_time;
        running = input_update(input_axis, mouse_pos);

        player_move(player, mouse_pos, input_axis, 50, 0.5f, delta, mesh_terrain);

        ecs_reset_entity(light_origin);
        ecs_translate_entity(light_origin,(vec3){50.0f, 10.0f, 50.0f});
        ecs_rotate_entity(light_origin,offset,(vec3) {0,1,0});
        offset+=delta * 100.0f;


        auto player_pos = ecs_get_position(player);
        auto player_rotation = ecs_get_rotation(player);
        auto player_scale = ecs_get_scale(player);

        // auto camera_rig_position = ecs_get_position(camera_rig);
        //
        // camera_rig_position->position[0] = player_pos.position[0];
        // camera_rig_position->position[1] = player_pos.position[1];
        // camera_rig_position->position[2] = player_pos.position[2];
      //  ecs_transform_entity(camera_rig,player_pos->position,(vec3) {1.0f,1.0f,1.0f},(vec3){0.0f,player_rotation->rotation[1],0.0f});
       //  ecs_run_update_system(delta);

        ecs_run_update_system(delta);



        graphics_begin();
        auto pos = ecs_get_world_position(light);
        auto player_pos1 = ecs_get_position(player);
        sprintf(front_log,"-- %f %f",player_pos1->position[0],player_pos1->position[2]);
        ecs_set_light(pos.position,player_pos1->position,(vec3){1.0f,0.9f,1.0f});
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
