#include <SDL2/SDL.h>
#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_mode_t.h>
#include <world.h>
#include "cglm/affine-pre.h"
#include "cglm/mat4.h"
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

#define MAX_CUBES 1000
int
main(void) {
    SDL_Init(SDL_INIT_EVERYTHING);
    auto window = SDL_CreateWindow("my test window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN| SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_ShowCursor(false);

    graphics_init(window);
    gui_init(window);

    g_shader shader;
    graphics_load_shaders(&shader, "assets/shader.vert", "assets/shader.frag");
    g_texture texture = graphics_load_texture("assets/marble2.jpg");

    g_vertex boxVertices[24] = {// Front face
                                {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
                                {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
                                {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                                {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

                                // Back face
                                {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
                                {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
                                {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
                                {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},

                                // Left face
                                {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
                                {{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
                                {{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
                                {{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

                                // Right face
                                {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
                                {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
                                {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
                                {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

                                // Top face
                                {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                                {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
                                {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
                                {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

                                // Bottom face
                                {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
                                {{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
                                {{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
                                {{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}};
    unsigned int boxIndices[36] = {// Front face
                                   0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4,
                                   // Left face
                                   8, 9, 10, 10, 11, 8,
                                   // Right face
                                   12, 13, 14, 14, 15, 12,
                                   // Top face
                                   16, 17, 18, 18, 19, 16,
                                   // Bottom face
                                   20, 21, 22, 22, 23, 20};
    g_texture textures[1] = {texture};
    auto mesh = graphics_create_mesh(24, 36, 1, boxVertices, boxIndices, textures);
    SDL_Event event;
    create_world();
    g_entity cubes[MAX_CUBES] = {};
    cubes[0] = world_create_entity("cube", (vec3){0, 0, -10}, (vec3){1, 1, 1}, 0, (vec3){0, 1, 0}, world);
    for (int i = 1; i < MAX_CUBES; i++) {
        char name[100] = {"cube"};

        char num [100] = {0}; 
        SDL_itoa(i, num, 10);
       // name[4] = i + '0';
       strcat(name,num);
        cubes[i] = world_create_entity(name, (vec3){-40 + (rand() % 40), -40 + (rand() % 40),  -40 + (rand() % 40)}, (vec3){1, 1, 1}, 0, (vec3){0, 1, 0},
                                       cubes[0].entity);
    }

    graphics_use_shader(&shader);
    float speed = 0.05f;
    float angle = 0;
    bool running = true;
    unsigned int old_time, current_time;
    Uint32 start_time = SDL_GetTicks();

    current_time = SDL_GetTicks();
    vec2 input_axis = {};
    player_t player = (player_t){.yaw = -90.0f, .pitch = 0, .pos[2] = 10};
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
        glm_vec3_scale(graphics_get_active_camera().front, 100* delta, forward);
        glm_vec3_scale(graphics_get_active_camera().right, 100 * delta, right);
        if (input_axis[1] >= 1) {
            glm_vec3_add(player.pos, forward, player.pos);
        }
        if (input_axis[1] <= -1) {
            glm_vec3_sub(player.pos, forward, player.pos);
        }
        if (input_axis[0] <= -1) {
            glm_vec3_sub(player.pos, right, player.pos);
        }
        if (input_axis[0] >= 1) {
            glm_vec3_add(player.pos, right, player.pos);
        }

        mat4* first_cube;

        world_get_local_transform(cubes[0], &first_cube);
        glm_mat4_identity(*first_cube);
        glm_rotate(*first_cube, radians(angle), (vec3){0, 1, 0});
        world_update(delta);
        graphics_begin();
        graphics_set_camera(player.pos, (vec3){0.0f, 1.0f, 0.0f}, player.yaw, player.pitch);

        for (int i = 0; i < MAX_CUBES; i++) {

            mat4* model;
            world_get_world_transform(cubes[i], &model);
            draw_mesh_transform(mesh, *model);
        }

        angle += 0.01f;
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
