#ifndef _WORLD_H
#define _WORLD_H
#include <cglm/cglm.h>
#include <flecs.h>
#include <graphics.h>
#include "flecs/addons/flecs_c.h"

typedef struct {
    mat4 matrix;
} g_transform;

typedef struct {
    ecs_entity_t entity;
    ecs_entity_t parent;
} g_entity;

extern ecs_entity_t world;
void create_world();
g_entity world_create_entity(const char* name, vec3 pos, vec3 scale, float angle, vec3 axis, ecs_entity_t parent);
void world_get_local_transform(g_entity e, mat4 **out);
void
world_get_world_transform(g_entity e, mat4 **out);
void world_update(float dt);
void world_destroy();
#endif
