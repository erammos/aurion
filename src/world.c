#include <world.h>
#include "cglm/affine-pre.h"
#include "flecs.h"
static ecs_world_t * ecs;
static ecs_entity_t root;
void create_world()
{
 ecs = ecs_init();
}

g_entity world_create_entity(const char * name, vec3 pos, vec3 scale,float angle, vec3 axis, ecs_entity_t parent)
{

    ECS_COMPONENT_DEFINE(ecs, g_transform);
	ECS_TAG(ecs,World);
	ECS_TAG(ecs,Local);
	ecs_entity_t e = ecs_entity(ecs, { .name = name });
	ecs_add_pair(ecs,e,ecs_id(g_transform), World);
	ecs_add_pair(ecs,e,ecs_id(g_transform), Local);
    ecs_add_pair(ecs, e, EcsChildOf, parent);
    auto p = ecs_get_mut_pair(ecs, e, g_transform, Local);
    glm_translate(p->matrix, (vec3){pos[0],pos[1],pos[2]}); 
    glm_rotate(p->matrix, angle, axis);
    glm_scale(p->matrix, scale);
    return (g_entity) {.entity = e, .parent = parent};
}
void word_add_entity(g_entity)
{

}
void world_update()
{

}