#ifndef _GRAPHICS_H
#define _GRAPHICS_H
#include <cglm/cglm.h>
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

g_mesh graphics_create_mesh(size_t num_vertices, size_t num_indices, size_t num_textures,
                            g_vertex vertices[static num_vertices], unsigned int indices[static num_indices],
                            g_texture textures[static num_textures]);
int graphics_init(void* window);
void graphics_destroy();
int graphics_load_shaders(g_shader* shader, const char* vs_file, const char* fs_file);
void graphics_draw_mesh(g_mesh* mesh);
void graphics_draw_begin();
void graphics_draw_end();
void graphics_use_shader(g_shader* shader);
void graphics_set_camera(vec3 pos, vec3 up, float yaw, float pitch);
g_texture graphics_load_texture(const char* path);
void graphics_set_transform(mat4 transform);
double degrees(double radians);
double radians(double degrees);

#endif
