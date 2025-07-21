#ifndef _WORLD_H
#define _WORLD_H
#include <cglm/cglm.h>
#include <flecs.h>
#include <graphics.h>
#include "flecs/addons/flecs_c.h"

typedef struct {
    ecs_entity_t entity;
    ecs_entity_t parent;
} g_entity;

extern ecs_entity_t world;
void ecs_init_systems();
g_entity ecs_create_entity(const char* name, vec3 pos, vec3 scale, vec3 rotate, ecs_entity_t parent);
void ecs_get_local_transform(g_entity e, mat4** out);
mat4 * ecs_get_world_transform(g_entity e);
void ecs_run_update_system(float dt);
void ecs_add_mesh(g_entity e, c_mesh* m);
void ecs_run_render_system();
void world_destroy();
void ecs_transform_entity(g_entity e, vec3 pos, vec3 scale, vec3 rotate);
c_position* ecs_get_position(g_entity e);
c_rotation* ecs_get_rotation(g_entity e);
c_scale* ecs_get_scale(g_entity e);
void ecs_translate_entity(g_entity e, vec3 pos);
void ecs_scale_entity(g_entity e, vec3 scale);
void ecs_rotate_entity(g_entity e, float angle, vec3 axis);
void ecs_reset_entity(g_entity e);
c_position ecs_get_world_position(g_entity e);
void ecs_set_light(vec3 pos, vec3 viewPos, vec3 lightColor);

#endif
