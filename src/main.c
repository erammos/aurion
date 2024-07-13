#include <SDL2/SDL.h>
#include <graphics.h>
#include <stdlib.h>
#include "cglm/affine-pre.h"
#include "cglm/mat4.h"

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

int
main(void) {
    SDL_Init(SDL_INIT_EVERYTHING);
    auto window = SDL_CreateWindow("my test window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    graphics_init(window);
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
    graphics_use_shader(&shader);
    graphics_set_camera((vec3){0, 0, 10}, (vec3){0.0f, 1.0f, 0.0f}, -90, 0);
    float speed = 0.05f;
    float angle = 0;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        graphics_draw_begin();

        mat4 parent = GLM_MAT4_IDENTITY_INIT;
        glm_rotate(parent, 4, (vec3){1, 0, 0});
        draw_mesh(mesh, parent, (vec3){0, 0, 0}, (vec3){1, 1, 1}, (vec3){0, 1, 0}, angle);
        //draw_mesh(mesh, parent,(vec3){0, 2, -0}, (vec3){1, 1, 1},(vec3){1, 0, 0},90);
        angle += 1.0 * speed;

        graphics_draw_end();
    }

    graphics_destroy();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
