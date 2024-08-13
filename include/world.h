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
void create_world();
g_entity world_create_entity(const char* name, vec3 pos, vec3 scale, vec3 rotate, ecs_entity_t parent);
void world_get_local_transform(g_entity e, mat4** out);
void world_get_world_transform(g_entity e, mat4** out);
void world_update(float dt);
void world_add_mesh(g_entity e, g_mesh* m);
void world_draw();
void world_destroy();
void world_transform_entity(g_entity e, vec3 pos, vec3 scale, vec3 rotate);
g_position * 
world_get_position(g_entity e); 

g_rotation * 
world_get_rotation(g_entity e);
g_scale * 
world_get_scale(g_entity e);
#endif
