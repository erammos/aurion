#pragma once
#include <cglm/cglm.h>
#include <graphics.h>
#include <flecs.h>
#include "flecs/addons/flecs_c.h"

typedef struct {
    mat4 matrix;
} g_transform;
typedef struct {
    ecs_entity_t entity;
    ecs_entity_t parent;
}g_entity;
ECS_COMPONENT_DECLARE(g_transform);
ECS_COMPONENT_DECLARE(g_mesh);
ECS_TAG_DECLARE(Local);
ECS_TAG_DECLARE(World);

void create_world();
g_entity world_create_entity(const char * name, vec3 pos, vec3 scale,float angle, vec3 axis, ecs_entity_t parent);
void word_add_entity(g_entity);
void world_update(float dt);
void world_destroy();





