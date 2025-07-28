#pragma once
#include "cglm/call/ivec3.h"
#include <flecs.h>


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
} c_texture;

typedef struct {
    g_vertex* vertices;
    unsigned int* indices;
    size_t num_v, num_i;
    unsigned int vao, vbo, ebo;
} c_mesh;

g_shader g_pbr_shader;
g_shader g_cel_shader;


typedef struct { c_texture texture; } c_diffuse_texture;
typedef struct { c_texture texture; } c_specular_texture;
typedef struct { c_texture texture; } c_normal_map;

typedef struct { vec4 color; } c_tint_color;
typedef struct { float shininess; } c_shininess;

typedef struct {vec3 pos; vec3 viewPos; vec3 lightColor; } g_light;
typedef struct emission {
    vec3 orbColor;
    float intensity;
    vec3 centerPosition;
    float radius;
} c_emission;

typedef struct {
    mat4 matrix;
} c_transform;
typedef struct {
    vec3 position;
} c_position;
typedef struct {
    vec3 rotation;
} c_rotation;
typedef struct {
    vec3 scale;
} c_scale;

typedef struct {
    mat4 projection;
} c_camera;

typedef struct {
    vec3 pos;
    mat4 view;
    mat4 projection;
} s_camera_data;



ECS_COMPONENT_DECLARE(c_transform);
ECS_COMPONENT_DECLARE(c_position);
ECS_COMPONENT_DECLARE(c_rotation);
ECS_COMPONENT_DECLARE(c_scale);
ECS_COMPONENT_DECLARE(c_mesh);
ECS_COMPONENT_DECLARE(c_texture);
ECS_COMPONENT_DECLARE(g_light);
ECS_COMPONENT_DECLARE(g_camera);
ECS_COMPONENT_DECLARE(c_camera);
ECS_COMPONENT_DECLARE(s_camera_data);
ECS_COMPONENT_DECLARE(c_emission);

ECS_TAG_DECLARE(World);
ECS_TAG_DECLARE(Local);
ECS_TAG_DECLARE(UsesPBRShader);
ECS_TAG_DECLARE(UsesEmissiveShader);