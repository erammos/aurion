#include <ecs.h>

#include "cglm/affine-pre.h"
#include "cglm/euler.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/util.h"
#include "defines.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include <glad/glad.h>
static ecs_world_t* ecs;
static ecs_entity_t update_system;
static ecs_entity_t pbr_render_system;
static ecs_entity_t emissive_render_system;
static ecs_entity_t skybox_render_system;



ecs_entity_t world;

void
update_transform(ecs_iter_t* it) {
    // Print delta_time. The same value is passed to all systems.
    c_transform* local = ecs_field(it, c_transform, 0);
    c_transform* world = ecs_field(it, c_transform, 1);
    c_transform* parent_world = ecs_field(it, c_transform, 2);

    const c_camera* cam = ecs_field(it, c_camera, 3);


    // Inner loop, iterates entities in archetype
    for (int i = 0; i < it->count; i++) {
        glm_mat4_copy(local[i].matrix, world[i].matrix);
        if (parent_world) {
            glm_mat4_mul(parent_world[i].matrix, world[i].matrix, world[i].matrix);
        }

        if (cam) {
            // Get a mutable pointer to the singleton s_camera_data
            const s_camera_data* active_cam = ecs_singleton_get(it->world, s_camera_data);

            // The view matrix is the inverse of the camera entity's final world transform
            glm_mat4_inv(world[i].matrix, active_cam->view);

            // Copy the projection matrix from the component to the singleton
            glm_mat4_copy(cam[i].projection, active_cam->projection);

            // Also store the camera's world position for lighting, etc.
            glm_mat4_mulv3(world[i].matrix, (vec3){0.0f, 0.0f, 0.0f}, 1.0f, active_cam->pos);
        }
        //     auto name = ecs_get_name(ecs, it->entities[i]);
        //    printf("%s:\n", name);
        //    glm_mat4_print(world[i].matrix,stdout);
    }
}

void
pbr_render(ecs_iter_t* it) {

    c_transform* model = ecs_field(it, c_transform, 0);
    c_mesh* mesh = ecs_field(it, c_mesh, 1);
    c_texture* texture = ecs_field(it, c_texture, 2);
    const g_light* light = ecs_singleton_get(it->world, g_light);
    const s_camera_data * camera = ecs_singleton_get(it->world, s_camera_data);
    graphics_use_shader(&g_pbr_shader);
    graphics_set_light(light->pos, light->viewPos, light->lightColor);
    graphics_set_uniform_mat4("view", camera->view);
    graphics_set_uniform_mat4("projection", camera->projection);

    for (int i = 0; i < it->count; i++) {
        graphics_set_transform(model[i].matrix);
        if (texture) {
            graphics_set_uniform_int("has_texture", 1);
            graphics_bind_texture(texture[i]);
        }
        else {
            graphics_set_uniform_int("has_texture", 0);
            graphics_set_uniform_vec3("default_color", (vec3){0.8f, 0.8f, 0.8f}); // A nice grey
        }
        graphics_draw_mesh(&mesh[i]);
    }
}

void
emissive_render(ecs_iter_t* it) {

    c_transform* model = ecs_field(it, c_transform, 0);
    c_mesh* mesh = ecs_field(it, c_mesh, 1);
    c_emission* emission = ecs_field(it, c_emission, 2);
    const s_camera_data * camera = ecs_singleton_get(it->world, s_camera_data);
    graphics_use_shader(&g_cel_shader);
    graphics_set_uniform_mat4("view", camera->view);
    graphics_set_uniform_mat4("projection", camera->projection);

    for (int i = 0; i < it->count; i++) {
        graphics_set_transform(model[i].matrix);
        graphics_set_uniform_float("intensity", emission[i].intensity);
        graphics_set_uniform_vec3("orbColor", emission[i].orbColor);
        graphics_set_uniform_float("radius", emission[i].radius);
        graphics_set_uniform_vec3("centerPosition", emission[i].centerPosition);
        graphics_draw_mesh(&mesh[i]);
    }
}


void skybox_render(ecs_iter_t* it) {
    c_mesh* mesh = ecs_field(it, c_mesh, 0);
    c_skybox* skybox = ecs_field(it, c_skybox, 1);

    const s_camera_data* camera = ecs_singleton_get(it->world, s_camera_data);

    // Change depth function so depth test passes when values are equal to depth buffer's content
    glDepthFunc(GL_LEQUAL);

    graphics_use_shader(&g_skybox_shader); // Assuming you load this shader into a global
    graphics_set_uniform_mat4("view", camera->view);
    graphics_set_uniform_mat4("projection", camera->projection);

    // Render the skybox cube
    for (int i = 0; i < it->count; i++) {
        glBindVertexArray(mesh[i].vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox[i].cubemap_id);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    glDepthFunc(GL_LESS); // Set depth function back to default
}


void
ecs_init_systems() {
    ecs = ecs_init();
    ECS_COMPONENT_DEFINE(ecs, c_transform);
    ECS_COMPONENT_DEFINE(ecs, c_mesh);
    ECS_COMPONENT_DEFINE(ecs, c_position);
    ECS_COMPONENT_DEFINE(ecs, c_rotation);
    ECS_COMPONENT_DEFINE(ecs, c_scale);
    ECS_COMPONENT_DEFINE(ecs, c_texture);
    ECS_COMPONENT_DEFINE(ecs, c_emission);
    ECS_COMPONENT_DEFINE(ecs, c_camera);
    ECS_COMPONENT_DEFINE(ecs, s_camera_data);
    ECS_COMPONENT_DEFINE(ecs, c_skybox);



    ECS_TAG_DEFINE(ecs, World);
    ECS_TAG_DEFINE(ecs, Local);
    ECS_TAG_DEFINE(ecs, UsesPBRShader);
    ECS_TAG_DEFINE(ecs, UsesEmissiveShader);
    update_system = ecs_system(
        ecs, {.entity = ecs_entity(ecs, {.name = "Update transform", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
              .query.terms =
                  {// Read from entity's Local position
                   {.id = ecs_pair(ecs_id(c_transform), Local), .inout = EcsIn},
                   // Write to entity's World position
                   {.id = ecs_pair(ecs_id(c_transform), World), .inout = EcsOut},

                   // Read from parent's World position
                   {.id = ecs_pair(ecs_id(c_transform), World),
                    .inout = EcsIn,
                    // Get from the parent in breadth-first order (cascade)
                    .src.id = EcsCascade,
                    .oper = EcsOptional

                   },
                   {.id = ecs_id(c_camera), .inout = EcsIn, .oper = EcsOptional}
                  },

              .callback = update_transform});

    pbr_render_system = ecs_system(
        ecs, {.entity = ecs_entity(ecs, {.name = "PBRRender", .add = ecs_ids(ecs_dependson(EcsPostFrame))}),
              .query.terms =
                  {
                      {.id = ecs_pair(ecs_id(c_transform), World), .inout = EcsIn},
                      {.id = ecs_id(c_mesh), .inout = EcsIn},
                       {.id = ecs_id(c_texture), .inout = EcsIn, .oper = EcsOptional},
                       {.id  = UsesPBRShader}
                  },
              .callback = pbr_render});

    emissive_render_system = ecs_system(
    ecs, {.entity = ecs_entity(ecs, {.name = "EmissiveRender", .add = ecs_ids(ecs_dependson(EcsPostFrame))}),
          .query.terms =
              {
                  {.id = ecs_pair(ecs_id(c_transform), World), .inout = EcsIn},
                  {.id = ecs_id(c_mesh), .inout = EcsIn},
                   {.id = ecs_id(c_emission), .inout = EcsIn},
                   {.id  = UsesEmissiveShader}
              },
          .callback = emissive_render});


    skybox_render_system = ecs_system(
        ecs, {.entity = ecs_entity(ecs, {.name = "SkyboxRender", .add = ecs_ids(ecs_dependson(EcsPreFrame))}), // Runs before other render systems
              .query.terms =
                  {
                      {.id = ecs_id(c_mesh)},
                      {.id = ecs_id(c_skybox)}
                  },
              .callback = skybox_render});

    world = ecs_entity(ecs, {.name = "root"});

    ecs_singleton_set(ecs, s_camera_data, {0});
}

void
destroy_ecs() {
    ecs_fini(ecs);
}

void
ecs_transform_entity(g_entity e, vec3 pos, vec3 scale, vec3 rotate) {
    auto p = ecs_get_mut_pair(ecs, e.entity, c_transform, Local);

    ecs_set(ecs, e.entity, c_position, {.position = {pos[0], pos[1], pos[2]}});
    ecs_set(ecs, e.entity, c_rotation, {.rotation = {rotate[0], rotate[1], rotate[2]}});
    ecs_set(ecs, e.entity, c_scale, {.scale = {scale[0], scale[1], scale[2]}});
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
ecs_translate_entity(g_entity e, vec3 pos) {
    auto p = ecs_get_mut_pair(ecs, e.entity, c_transform, Local);
    glm_translate(p->matrix, pos);
}

void
ecs_scale_entity(g_entity e, vec3 scale) {
    auto p = ecs_get_mut_pair(ecs, e.entity, c_transform, Local);
    glm_scale(p->matrix, scale);
}

void
ecs_rotate_entity(g_entity e, float angle, vec3 axis) {
    auto p = ecs_get_mut_pair(ecs, e.entity, c_transform, Local);
    glm_rotate(p->matrix, glm_rad(angle), axis);
}

void
ecs_reset_entity(g_entity e) {

    auto p = ecs_get_mut_pair(ecs, e.entity, c_transform, Local);
    glm_mat4_identity(p->matrix);
}

g_entity
ecs_create_entity(const char* name, vec3 pos, vec3 scale, vec3 rotate, ecs_entity_t parent) {

    ecs_entity_t e = ecs_entity(ecs, {.name = name});
    ecs_set(ecs, e, c_position, {.position = {pos[0], pos[1], pos[2]}});
    ecs_set(ecs, e, c_rotation, {.rotation = {rotate[0], rotate[1], rotate[2]}});
    ecs_set(ecs, e, c_scale, {.scale = {scale[0], scale[1], scale[2]}});
    ecs_set_pair(ecs, e, c_transform, World, {GLM_MAT4_IDENTITY_INIT});
    ecs_set_pair(ecs, e, c_transform, Local, {GLM_MAT4_IDENTITY_INIT});
    ecs_add_pair(ecs, e, EcsChildOf, parent);
    auto out = (g_entity){.entity = e, .parent = parent};
    ecs_transform_entity(out, pos, scale, rotate);
    return out;
}

void
ecs_add_mesh(g_entity e, c_mesh* m) {
    ecs_set_ptr(ecs, e.entity, c_mesh, m);
}
void ecs_use_pbr_shader(g_entity e) {
    ecs_add(ecs,e.entity, UsesPBRShader);
}
void ecs_use_emissive_shader(g_entity e, c_emission emission) {
    ECS_COMPONENT_DEFINE(ecs, c_emission);
    ecs_add(ecs, e.entity, UsesEmissiveShader);

    ecs_set(ecs, e.entity, c_emission, {
        .centerPosition = {emission.centerPosition[0], emission.centerPosition[1], emission.centerPosition[2]},
        .intensity = emission.intensity,
        .orbColor = {emission.orbColor[0], emission.orbColor[1], emission.orbColor[2]},
        .radius = emission.radius}
    );
}

void ecs_add_texture(g_entity e, c_texture* t) {
    ecs_set_ptr(ecs, e.entity, c_texture, t);
}

void ecs_set_light(vec3 pos, vec3 viewPos, vec3 lightColor)
 {
    ECS_COMPONENT_DEFINE(ecs, g_light);
    ecs_singleton_set(ecs,g_light, {.pos = {pos[0], pos[1], pos[2]}, .lightColor = {lightColor[0], lightColor[1], lightColor[2]} , .viewPos = {viewPos[0], viewPos[1], viewPos[2]}});
 }

void ecs_add_camera(g_entity e,float aspectRatio) {
   ECS_COMPONENT_DEFINE(ecs,c_camera);
    mat4 projection_matrix;
    glm_perspective(DEG2RAD(45), aspectRatio, 0.1f, 1000,projection_matrix);

    ecs_set(ecs, e.entity,c_camera, {
     .projection = {
         {projection_matrix[0][0], projection_matrix[0][1], projection_matrix[0][2], projection_matrix[0][3]},
         {projection_matrix[1][0], projection_matrix[1][1], projection_matrix[1][2], projection_matrix[1][3]},
         {projection_matrix[2][0], projection_matrix[2][1], projection_matrix[2][2], projection_matrix[2][3]},
         {projection_matrix[3][0], projection_matrix[3][1], projection_matrix[3][2], projection_matrix[3][3]}
     }
 });
}

void ecs_add_skybox(g_entity e, unsigned int skybox_texture) {
    ECS_COMPONENT_DEFINE(ecs, c_skybox);
    c_mesh skybox_mesh = graphics_create_skybox_mesh();
     ecs_add_mesh(e,&skybox_mesh);
    ecs_set(ecs,e.entity,c_skybox, { .cubemap_id = skybox_texture });
}
void
ecs_get_local_transform(g_entity e, mat4** out) {
    auto p = ecs_get_mut_pair(ecs, e.entity, c_transform, Local);
    *out = &p->matrix;
}

mat4 *
ecs_get_world_transform(g_entity e) {
    auto p = ecs_get_mut_pair(ecs, e.entity, c_transform, World);
    return p->matrix;
}

c_position*
ecs_get_position(g_entity e) {
    auto p = ecs_get_mut(ecs, e.entity, c_position);
    return p;
}

c_position
ecs_get_world_position(g_entity e) {
    auto p = ecs_get_mut(ecs, e.entity, c_position);
    auto  transform = ecs_get_world_transform(e);
    c_position out = {0};
    glm_mat4_mulv3(*transform, p->position,1,out.position) ;
    return out;
}

c_rotation*
ecs_get_rotation(g_entity e) {
    auto p = ecs_get_mut(ecs, e.entity, c_rotation);
    return p;
}

c_rotation*
ecs_get_world_rotation(g_entity e) {
    auto p = ecs_get_mut(ecs, e.entity, c_rotation);
    return p;
}


c_scale*
ecs_get_scale(g_entity e) {
    auto p = ecs_get_mut(ecs, e.entity, c_scale);
    return p;
}

void
ecs_run_render_system() {
    ecs_run(ecs, skybox_render_system, 0, 0);
    ecs_run(ecs, pbr_render_system, 0, 0);
    ecs_run(ecs, emissive_render_system, 0, 0);

}

void
ecs_run_update_system(float dt) {
    ecs_run(ecs, update_system, 0, 0);
}