#include <SDL2/SDL.h>
#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_mode_t.h>
#include <world.h>
#include "camera.h"
#include "cglm/affine-pre.h"
#include "cglm/io.h"
#include "cglm/mat4.h"
#include "cglm/util.h"
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

void
generate_circle(vec3* circle, int segments, float radius) {
    float angleStep = 2.0f * M_PI / segments;
    for (int i = 0; i < segments; ++i) {
        float angle = i * angleStep;
        circle[i][0] = cosf(angle) * radius;
        circle[i][1] = sinf(angle) * radius;
        circle[i][2] = 0.0f; // Circle lies in the XY plane
    }
}

g_mesh
generate_tunnel(int segments, int length, float radius) {
    vec3* circle = (vec3*)malloc(segments * sizeof(vec3));
    generate_circle(circle, segments, radius);

    g_mesh mesh;
    mesh.num_v = segments * length;
    mesh.num_i = segments * (length - 1) * 6;
    mesh.vertices = (g_vertex*)malloc(mesh.num_v * sizeof(g_vertex));
    mesh.indices = (unsigned int*)malloc(mesh.num_i * sizeof(unsigned int));
    mesh.num_t = 1;
    mesh.textures = (g_texture*)malloc(mesh.num_t * sizeof(g_texture));
    mesh.textures[0] = graphics_load_texture("assets/marble2.jpg");
    int vertexCount = 0;
    int indexCount = 0;

    for (int i = 0; i < length; ++i) {
        for (int j = 0; j < segments; ++j) {
            vec3 position;
            glm_vec3_copy(circle[j], position);
            position[2] = (float)i; // Move circle along Z axis

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

g_mesh
create_orb_mesh(float radius, int sectors, int stacks) {
    int numVertices = (sectors + 1) * (stacks + 1);
    int numIndices = sectors * stacks * 6;
    g_vertex* vertices = malloc(sizeof(g_vertex) * numVertices);
    unsigned int* indices = malloc(sizeof(unsigned int) * numIndices);

    float x, y, z, xy;                           // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius; // normal
    float s, t;                                  // texCoord

    float sectorStep = 2 * M_PI / sectors;
    float stackStep = M_PI / stacks;
    float sectorAngle, stackAngle;

    int k = 0;
    for (int i = 0; i <= stacks; ++i) {
        stackAngle = M_PI / 2 - i * stackStep; // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);        // r * cos(u)
        z = radius * sinf(stackAngle);         // r * sin(u)

        for (int j = 0; j <= sectors; ++j, ++k) {
            sectorAngle = j * sectorStep; // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)
            glm_vec3_copy((vec3){x, y, z}, vertices[k].position);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            glm_vec3_copy((vec3){nx, ny, nz}, vertices[k].normal);

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectors;
            t = (float)i / stacks;
            glm_vec2_copy((vec2){s, t}, vertices[k].uv);
        }
    }

    int index = 0;
    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1); // beginning of current stack
        int k2 = k1 + sectors + 1;  // beginning of next stack

        for (int j = 0; j < sectors; ++j, ++index) {
            if (i != 0) {
                indices[index++] = k1 + j;
                indices[index++] = k2 + j;
                indices[index++] = k1 + j + 1;
            }

            if (i != (stacks - 1)) {
                indices[index++] = k1 + j + 1;
                indices[index++] = k2 + j;
                indices[index++] = k2 + j + 1;
            }
        }
    }

    g_mesh orbMesh = graphics_create_mesh(numVertices, numIndices, 0, vertices, indices, nullptr);

    free(vertices);
    free(indices);

    return orbMesh;
}
char front_log[100] = {0};
char rot_log[100] = {0};
void
player_move(g_entity e, vec3 mouse_pos, vec3 input_axis, float speed, float sensitivity, float dt) {

    mat4* matrix;
    world_get_local_transform(e, &matrix);

    g_rotation* g_rotation = world_get_rotation(e);
    vec3 front, right;
    front[0] = cos(glm_rad(g_rotation->rotation[1])) * cos(glm_rad(g_rotation->rotation[0]));
    front[1] = sin(glm_rad(g_rotation->rotation[0]));
    front[2] = sin(glm_rad(g_rotation->rotation[1])) * cos(glm_rad(g_rotation->rotation[0]));
    glm_normalize(front);
    glm_vec3_cross((vec3){0.0, 1.0, 0.0f}, front, right);
    glm_vec3_normalize(right);
    sprintf(front_log, "%f %f %f", front[0],front[1],front[2]);
    sprintf(rot_log, "%f %f %f", g_rotation->rotation[0],g_rotation->rotation[1],g_rotation->rotation[2]);
    glm_vec3_scale(front, speed * dt * input_axis[1], front);
    glm_vec3_scale(right, speed * dt * input_axis[0], right);
    g_position* g_pos = world_get_position(e);
    g_scale* g_scale = world_get_scale(e);

    front[1] = 0;
    glm_vec3_add(g_pos->position, front, g_pos->position);

    g_rotation->rotation[1] += sensitivity * mouse_pos[0];
    g_rotation->rotation[0] -= sensitivity * mouse_pos[1];
    g_rotation->rotation[2] = 0;
    world_transform_entity(e, g_pos->position, g_scale->scale, g_rotation->rotation);
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
    auto player = world_create_entity("player", (vec3){50, 0.5f, 50}, (vec3){1.0f, 01.0f, 1.0f}, (vec3){0, 0, 0},
                                      world);
    auto e_camera = world_create_entity("camera", (vec3){0, 0, 0}, (vec3){1, 1, 1}, (vec3){0, 0, 0}, terrain.entity);

    auto mesh_obj = graphics_load_obj("assets/test.obj");
    //   world_add_mesh(cube, &mesh_obj);
    world_add_mesh(player, &mesh_obj);

    SDL_Event event;
    bool running = true;
    unsigned int old_time, current_time;
    Uint32 start_time = SDL_GetTicks();

    current_time = SDL_GetTicks();
    vec3 input_axis = {};
    g_mesh tunnel = generate_tunnel(10, 1000, 10);
    g_mesh orb = create_orb_mesh(1, 10, 10);
    vec3 mouse_pos = {0};
    int frame_count = 0;
    char fps[10] = {0};
    g_camera camera = camera_create(graphics_get_width(), graphics_get_height());
    camera_update(&camera, (vec3){50, 1.0f, 50 - 5}, (vec3){0, 1, 0}, 180, 0);

    while (running) {
        current_time = SDL_GetTicks();
        float delta = (float)(current_time - old_time) / 1000.0f;
        running = input_update(input_axis, mouse_pos);

        //world_transform_entity(g_entity e, float *pos, float *scale, float *rotate)

        // camera.pos[1] = mesh_terrain.vertices[(int)camera.pos[0] + 100 * (int)camera.pos[2]].position[1] + 2;
        //        camera_locked_animate(&camera, mouse_pos, input_axis, 10, 0.5f, delta);
        player_move(player, mouse_pos, input_axis, 10, 0.5f, delta);
        // mat4 * player_model;
        // world_get_local_transform(player, &player_model);
        //  glm_mat4_inv(*player_model,camera.view);
        // glm_mat4_copy(camera.view, *camera_model);

        world_update(delta);
        graphics_begin();
        graphics_use_shader(&light_shader);

        graphics_use_camera(&camera);
        world_draw();
        // mat4 model_terrain = GLM_MAT4_IDENTITY_INIT;
        // glm_translate(model_terrain, (vec3){-100 / 2, -3, -100 / 2});
        // draw_mesh_transform(mesh_terrain, model_terrain);
        // draw_mesh_transform(mesh_obj, model_terrain);
        // draw_mesh_transform(tunnel, model_terrain);

        // graphics_use_shader(&orb_shader);
        // graphics_use_camera(&camera);
        // graphics_set_uniform_vec3("orbColor", (vec3){0.8f, 0.5f, 0.0f});       // Set orb color
        // graphics_set_uniform_float("intensity", 1000.0f);                      // Set intensity
        // graphics_set_uniform_vec3("centerPosition", (vec3){0.5f, 0.0f, 0.0f}); // Orb's position
        // graphics_set_uniform_float("radius", 2);

        // mat4 orb_matrix = GLM_MAT4_IDENTITY_INIT;
        // draw_mesh_transform(mesh_obj, orb_matrix);
        gui_begin();

        gui_draw_text(graphics_get_width() / 2, graphics_get_height() / 2, "+");

        gui_draw_text(10, 10, fps);
        gui_draw_text(10,50,front_log);
        gui_draw_text(10,90,rot_log);
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
