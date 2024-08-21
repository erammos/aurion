#include <world.h>
#include "cglm/affine-pre.h"
#include "cglm/euler.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/util.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "graphics.h"
static ecs_world_t* ecs;
static ecs_entity_t update_system;
static ecs_entity_t render_system;

ECS_COMPONENT_DECLARE(g_transform);
ECS_COMPONENT_DECLARE(g_position);
ECS_COMPONENT_DECLARE(g_rotation);
ECS_COMPONENT_DECLARE(g_scale);
ECS_COMPONENT_DECLARE(g_mesh);
ECS_TAG_DECLARE(World);
ECS_TAG_DECLARE(Local);
ecs_entity_t world;

void
update_transform(ecs_iter_t* it) {
    // Print delta_time. The same value is passed to all systems.
    g_transform* local = ecs_field(it, g_transform, 0);
    g_transform* world = ecs_field(it, g_transform, 1);
    g_transform* parent_world = ecs_field(it, g_transform, 2);

    // Inner loop, iterates entities in archetype
    for (int i = 0; i < it->count; i++) {
        glm_mat4_copy(local[i].matrix, world[i].matrix);
        if (parent_world) {
            glm_mat4_mul(parent_world[i].matrix, world[i].matrix, world[i].matrix);
        }
        //     auto name = ecs_get_name(ecs, it->entities[i]);
        //    printf("%s:\n", name);
        //    glm_mat4_print(world[i].matrix,stdout);
    }
}

void
render(ecs_iter_t* it) {
    g_transform* model = ecs_field(it, g_transform, 0);
    g_mesh* mesh = ecs_field(it, g_mesh, 1);

    for (int i = 0; i < it->count; i++) {
        graphics_set_transform(model[i].matrix);
        graphics_draw_mesh(&mesh[i]);
    }
}

void
create_world() {
    ecs = ecs_init();
    ECS_COMPONENT_DEFINE(ecs, g_transform);
    ECS_COMPONENT_DEFINE(ecs, g_mesh);
    ECS_COMPONENT_DEFINE(ecs, g_position);
    ECS_COMPONENT_DEFINE(ecs, g_rotation);
    ECS_COMPONENT_DEFINE(ecs, g_scale);

    ECS_TAG_DEFINE(ecs, World);
    ECS_TAG_DEFINE(ecs, Local);
    update_system = ecs_system(
        ecs, {.entity = ecs_entity(ecs, {.name = "Update transform", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
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

    render_system = ecs_system(
        ecs, {.entity = ecs_entity(ecs, {.name = "Render", .add = ecs_ids(ecs_dependson(EcsPostFrame))}),
              .query.terms =
                  {
                      {.id = ecs_pair(ecs_id(g_transform), World), .inout = EcsIn},
                      {.id = ecs_id(g_mesh), .inout = EcsIn},
                  },
              .callback = render});
    world = ecs_entity(ecs, {.name = "root"});
}

void
destroy_world() {
    ecs_fini(ecs);
}

void
world_transform_entity(g_entity e, vec3 pos, vec3 scale, vec3 rotate) {
    auto p = ecs_get_mut_pair(ecs, e.entity, g_transform, Local);

    ecs_set(ecs, e.entity, g_position, {.position = {pos[0], pos[1], pos[2]}});
    ecs_set(ecs, e.entity, g_rotation, {.rotation = {rotate[0], rotate[1], rotate[2]}});
    ecs_set(ecs, e.entity, g_scale, {.scale = {scale[0], scale[1], scale[2]}});
    mat4 rotation_mat;
    mat4 scale_mat;
    mat4 translation_mat;
    glm_translate_make(translation_mat, (vec3){pos[0], pos[1], pos[2]});
    glm_euler((vec3){glm_rad(rotate[0]), glm_rad(rotate[1]), glm_rad(rotate[2])}, rotation_mat);
    glm_scale_make(scale_mat, scale);
    glm_mul(rotation_mat, scale_mat, p->matrix);
    glm_mul(translation_mat, p->matrix, p->matrix);
}

void
world_translate_entity(g_entity e, vec3 pos) {
    auto p = ecs_get_mut_pair(ecs, e.entity, g_transform, Local);
    glm_translate(p->matrix, pos);
}

void
world_scale_entity(g_entity e, vec3 scale) {
    auto p = ecs_get_mut_pair(ecs, e.entity, g_transform, Local);
    glm_scale(p->matrix, scale);
}

void
world_rotate_entity(g_entity e, float angle, vec3 axis) {
    auto p = ecs_get_mut_pair(ecs, e.entity, g_transform, Local);
    glm_rotate(p->matrix, glm_rad(angle), axis);
}

void
world_reset_entity(g_entity e) {

    auto p = ecs_get_mut_pair(ecs, e.entity, g_transform, Local);
    glm_mat4_identity(p->matrix);
}

g_entity
world_create_entity(const char* name, vec3 pos, vec3 scale, vec3 rotate, ecs_entity_t parent) {

    ecs_entity_t e = ecs_entity(ecs, {.name = name});
    ecs_set(ecs, e, g_position, {.position = {pos[0], pos[1], pos[2]}});
    ecs_set(ecs, e, g_rotation, {.rotation = {rotate[0], rotate[1], rotate[2]}});
    ecs_set(ecs, e, g_scale, {.scale = {scale[0], scale[1], scale[2]}});
    ecs_set_pair(ecs, e, g_transform, World, {GLM_MAT4_IDENTITY_INIT});
    ecs_set_pair(ecs, e, g_transform, Local, {GLM_MAT4_IDENTITY_INIT});
    ecs_add_pair(ecs, e, EcsChildOf, parent);
    auto out = (g_entity){.entity = e, .parent = parent};
    world_transform_entity(out, pos, scale, rotate);
    return out;
}

void
world_add_mesh(g_entity e, g_mesh* m) {
    // ecs_add(ecs, e.entity, g_mesh);
    ecs_set_ptr(ecs, e.entity, g_mesh, m);
}

void
world_get_local_transform(g_entity e, mat4** out) {
    auto p = ecs_get_mut_pair(ecs, e.entity, g_transform, Local);
    *out = &p->matrix;
}

void
world_get_world_transform(g_entity e, mat4** out) {
    auto p = ecs_get_mut_pair(ecs, e.entity, g_transform, World);
    *out = &p->matrix;
}

g_position*
world_get_position(g_entity e) {
    auto p = ecs_get_mut(ecs, e.entity, g_position);
    return p;
}

g_rotation*
world_get_rotation(g_entity e) {
    auto p = ecs_get_mut(ecs, e.entity, g_rotation);
    return p;
}

g_scale*
world_get_scale(g_entity e) {
    auto p = ecs_get_mut(ecs, e.entity, g_scale);
    return p;
}

void
world_draw() {
    ecs_run(ecs, render_system, 0, 0);
}

void
world_update(float dt) {
    ecs_run(ecs, update_system, 0, 0);
}