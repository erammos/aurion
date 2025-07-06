#pragma once
#include <cglm/cglm.h>
#include "camera.h"
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

typedef struct {
    unsigned int id;

} g_shader;

typedef struct {
    vec3 position;
    vec3 normal;
    vec2 uv;

} g_vertex;

typedef struct {
    unsigned int id;
    const char* type;
} g_texture;

typedef struct {
    g_vertex* vertices;
    unsigned int* indices;
    g_texture* textures;
    size_t num_v, num_i, num_t;
    unsigned int vao, vbo, ebo;
} g_mesh;

typedef struct {
    mat4 matrix;
} g_transform;
typedef struct {
    vec3 position; 
} g_position;
typedef struct {
    vec3 rotation; 
} g_rotation;
typedef struct {
    vec3 scale; 
} g_scale;

double degrees(double radians);
double radians(double degrees);

g_mesh graphics_create_mesh(size_t num_vertices, size_t num_indices, size_t num_textures,
                            g_vertex vertices[static num_vertices], unsigned int indices[static num_indices],
                            g_texture* textures);
int graphics_init(void* window);
void graphics_destroy();
g_shader graphics_load_shaders(const char* vs_file, const char* fs_file);
void graphics_draw_mesh(g_mesh* mesh);
void graphics_begin();
void graphics_end();
void graphics_use_shader(g_shader* shader);
void graphics_use_camera(g_camera* camera);
g_texture graphics_load_texture(const char* path);
void graphics_set_transform(mat4 transform);
g_camera * 
graphics_get_active_camera();
int graphics_get_width();
int graphics_get_height();
g_mesh graphics_load_obj(const char* path);
void graphics_set_light(vec3 pos, vec3 viewPos);
g_mesh graphics_create_terrain(int terrain_width, int terrain_height);
void graphics_create_gl_buffer(g_mesh* mesh);
void graphics_set_uniform_vec3(const char* name, vec3 vec);
void graphics_set_uniform_float(const char* name, float value);
void graphics_set_uniform_mat4(const char* name, mat4 matrix);
void graphics_set_uniform_int(const char* name, int value);
