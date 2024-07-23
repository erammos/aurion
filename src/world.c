#include <world.h>
#include "cglm/affine-pre.h"
#include "cglm/mat4.h"
#include "flecs.h"
static ecs_world_t* ecs;
static ecs_entity_t update_system;

ECS_COMPONENT_DECLARE(g_transform);
ECS_COMPONENT_DECLARE(g_mesh);
ECS_TAG_DECLARE(World);
ECS_TAG_DECLARE(Local);
ecs_entity_t world;

void
update_transform(ecs_iter_t* it) {
    // Print delta_time. The same value is passed to all systems.
    g_transform* p = ecs_field(it, g_transform, 0);
    g_transform* p_out = ecs_field(it, g_transform, 1);
    g_transform* p_parent = ecs_field(it, g_transform, 2);

    // Inner loop, iterates entities in archetype
    for (int i = 0; i < it->count; i++) {
        glm_mat4_copy(p->matrix, p_out->matrix);
        if (p_parent) {
            glm_mat4_mul(p_parent->matrix, p_out->matrix, p_out->matrix);
        }
    }
}

void
create_world() {
    ecs = ecs_init();
    ECS_COMPONENT_DEFINE(ecs, g_transform);
    ECS_TAG_DEFINE(ecs, World);
    ECS_TAG_DEFINE(ecs, Local);
    update_system = ecs_system(
        ecs, {// Systems are entities, and by initializing the .entity field we can
              // set some additional properties for the system like a name. While this
              // is not mandatory, it makes a system easier to find in tools like the
              // explorer (https://www.flecs.dev/explorer/).
              .entity = ecs_entity(ecs, {.name = "Update transform", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
              .query.terms =
                  {// Read from entity's Local position
                   {.id = ecs_pair(ecs_id(g_transform), Local), .inout = EcsIn},
                   // Write to entity's World position
                   {.id = ecs_pair(ecs_id(g_transform), World), .inout = EcsOut},

                   // Read from parent's World position
                   {.id = ecs_pair(ecs_id(g_transform), World),
                    .inout = EcsIn,
                    // Get from the parent in breadth-first order (cascade)
                    .src.id = EcsCascade,
                    .oper = EcsOptional

                   }},
              .callback = update_transform});
    world = ecs_entity(ecs, {.name = "root"});
}

void
destroy_world() {
    ecs_fini(ecs);
}

void
world_transform_entity(g_entity e, vec3 pos, vec3 scale, float angle, vec3 axis) {
    auto p = ecs_get_mut_pair(ecs, e.entity, g_transform, Local);
    glm_translate(p->matrix, (vec3){pos[0], pos[1], pos[2]});
    glm_rotate(p->matrix, angle, axis);
    glm_scale(p->matrix, scale);
}

g_entity
world_create_entity(const char* name, vec3 pos, vec3 scale, float angle, vec3 axis, ecs_entity_t parent) {

    ecs_entity_t e = ecs_entity(ecs, {.name = name});
    ecs_set_pair(ecs, e, g_transform, World, {GLM_MAT4_IDENTITY_INIT});
    ecs_set_pair(ecs, e, g_transform, Local, {GLM_MAT4_IDENTITY_INIT});
    ecs_add_pair(ecs, e, EcsChildOf, parent);
    auto out = (g_entity){.entity = e, .parent = parent};
    world_transform_entity(out, pos, scale, angle, axis);
    return out;
}

void
world_get_transform(g_entity e, mat4 **out) {
    auto p = ecs_get_mut_pair(ecs, e.entity, g_transform, World);
    *out = &p->matrix;
}

void
world_update(float dt) {
    ecs_progress(ecs, 0);
}