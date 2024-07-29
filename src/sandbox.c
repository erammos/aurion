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
#include "input.h"

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
void generate_circle(vec3* circle, int segments, float radius) {
    float angleStep = 2.0f * M_PI / segments;
    for (int i = 0; i < segments; ++i) {
        float angle = i * angleStep;
        circle[i][0] = cosf(angle) * radius;
        circle[i][1] = sinf(angle) * radius;
        circle[i][2] = 0.0f;  // Circle lies in the XY plane
    }
}

g_mesh generate_tunnel(int segments, int length, float radius) {
    vec3* circle = (vec3*)malloc(segments * sizeof(vec3));
    generate_circle(circle, segments, radius);

    g_mesh mesh;
    mesh.num_v = segments * length;
    mesh.num_i = segments * (length - 1) * 6;
    mesh.vertices = (g_vertex*)malloc(mesh.num_v * sizeof(g_vertex));
    mesh.indices = (unsigned int*)malloc(mesh.num_i * sizeof(unsigned int));
    mesh.num_t = 1;
    mesh.textures = (g_texture*)malloc(mesh.num_t * sizeof(g_texture));
    mesh.textures[0]= graphics_load_texture("assets/marble2.jpg");
    int vertexCount = 0;
    int indexCount = 0;

    for (int i = 0; i < length; ++i) {
        for (int j = 0; j < segments; ++j) {
            vec3 position;
            glm_vec3_copy(circle[j], position);
            position[2] = (float)i;  // Move circle along Z axis

            glm_vec3_copy(position, mesh.vertices[vertexCount].position);
            glm_vec3_normalize_to(position, mesh.vertices[vertexCount].normal);
            mesh.vertices[vertexCount].uv[0] = (float)j / segments;
            mesh.vertices[vertexCount].uv[1] = (float)i / length;

            if (i < length - 1) {
                int nextSegment = (j + 1) % segments;
                int currentIndex = i * segments + j;
                int nextIndex = currentIndex + segments;
                int nextSegmentIndex = i * segments + nextSegment;
                int nextSegmentNextIndex = nextSegmentIndex + segments;

                // First triangle
                mesh.indices[indexCount++] = currentIndex;
                mesh.indices[indexCount++] = nextIndex;
                mesh.indices[indexCount++] = nextSegmentNextIndex;

                // Second triangle
                mesh.indices[indexCount++] = currentIndex;
                mesh.indices[indexCount++] = nextSegmentNextIndex;
                mesh.indices[indexCount++] = nextSegmentIndex;
            }
            vertexCount++;
        }
    }

    free(circle);
  graphics_create_gl_buffer(&mesh);
    return mesh; 
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
    g_mesh tunnel = generate_tunnel(10,1000, 10);
    int mouse_pos[2] = {0};
    graphics_camera_perspective();
    int frame_count = 0;
    char fps[10] = {0};
    while (running) {
        current_time = SDL_GetTicks();
        float delta = (float)(current_time - old_time) / 1000.0f;
        running = input_update(input_axis, mouse_pos);
        player.yaw += 0.5f * mouse_pos[0];
        player.pitch -= 0.5f * mouse_pos[1];

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
        draw_mesh_transform(tunnel, model_terrain);
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
