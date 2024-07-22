#include <world.h>
#include "cglm/affine-pre.h"
#include "flecs.h"
static ecs_world_t * ecs;
static ecs_entity_t update_system;
void update_transform(ecs_iter_t *it) {
    // Print delta_time. The same value is passed to all systems.
     g_transform *p = ecs_field(it, g_transform, 0);
        g_transform *p_out = ecs_field(it, g_transform, 1);
        g_transform *p_parent = ecs_field(it, g_transform, 2);
        
        // Inner loop, iterates entities in archetype
        for (int i = 0; i < it->count; i ++) {
            glm_mat4_copy(p->matrix, p_out->matrix);
            if (p_parent) {
                glm_mat4_mul(p_parent->matrix, p_out->matrix, p_out->matrix);
            }
        }
}
void create_world()
{
 ecs = ecs_init();
      update_system = ecs_system(ecs, {
        // Systems are entities, and by initializing the .entity field we can
        // set some additional properties for the system like a name. While this
        // is not mandatory, it makes a system easier to find in tools like the
        // explorer (https://www.flecs.dev/explorer/).
        .entity = ecs_entity(ecs, {
            .name = "Update transform" 
        }),
        .query.terms = {
         // Read from entity's Local position
            { .id = ecs_pair(ecs_id(g_transform), Local), .inout = EcsIn }, 
            // Write to entity's World position
            { .id = ecs_pair(ecs_id(g_transform), World), .inout = EcsOut },

            // Read from parent's World position
            {
                .id = ecs_pair(ecs_id(g_transform), World), 
                .inout = EcsIn,
                // Get from the parent in breadth-first order (cascade)
                .src.id = EcsCascade,
                // Make parent term optional so we also match the root (sun)
                .oper = EcsOptional
            } 
        },
        .callback = update_transform
    });
}
void destroy_world()
{
    ecs_fini(ecs);
}

void world_transform_entity(g_entity e, vec3 pos, vec3 scale, float angle, vec3 axis)
{
    auto p = ecs_get_mut_pair(ecs, e.entity, g_transform, Local);
    glm_translate(p->matrix, (vec3){pos[0],pos[1],pos[2]}); 
    glm_rotate(p->matrix, angle, axis);
    glm_scale(p->matrix, scale);
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
    auto out = (g_entity) {.entity = e, .parent = parent};
    world_transform_entity(out, pos, scale, angle, axis);
    return out;

}
void word_add_entity(g_entity)
{


}
void world_update(float dt)
{
ecs_progress(ecs, 0);
}