#pragma once
#include <cglm/cglm.h>
#include "camera.h"
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif
#include <components.h>


double degrees(double radians);
double radians(double degrees);

c_mesh graphics_create_mesh(size_t num_vertices, size_t num_indices,
                            g_vertex vertices[static num_vertices], unsigned int indices[static num_indices]);
int graphics_init(void* window);
void graphics_destroy();
g_shader graphics_load_shaders(const char* vs_file, const char* fs_file);
void graphics_draw_mesh(c_mesh* mesh);
void graphics_begin();
void graphics_end();
void graphics_use_shader(g_shader* shader);
void graphics_use_camera(g_camera* camera);
c_texture graphics_load_texture(const char* path);
void graphics_set_transform(mat4 transform);
g_camera * 
graphics_get_active_camera();
int graphics_get_width();
int graphics_get_height();
c_mesh graphics_load_model(const char* path,c_texture* texture);
void graphics_set_light(vec3 pos, vec3 viewPos, vec3 lightColor);
c_mesh graphics_create_terrain(int terrain_width, int terrain_height);
void graphics_create_gl_buffer(c_mesh* mesh);
void graphics_set_uniform_vec3(const char* name, vec3 vec);
void graphics_set_uniform_float(const char* name, float value);
void graphics_set_uniform_mat4(const char* name, mat4 matrix);
void graphics_set_uniform_int(const char* name, int value);
void graphics_bind_texture(c_texture tex);
unsigned int graphics_load_cubemap(const char** faces);
c_mesh graphics_create_skybox_mesh();

