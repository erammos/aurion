#include <cglm/cglm.h>
#include <flecs.h>
#include "cglm/affine-pre.h"
#include "cglm/call/io.h"
#include "cglm/mat4.h"
#include "flecs/addons/flecs_c.h"
#include "minunit.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    mat4 matrix;
} Transform;

ECS_COMPONENT_DECLARE(Transform);
ECS_TAG_DECLARE(Local);
ECS_TAG_DECLARE(World);

void
test_setup(void) {}

void
test_teardown(void) {
    /* Nothing */
}

MU_TEST(test_check) {

    ecs_world_t* ecs = ecs_init();
    ECS_COMPONENT_DEFINE(ecs, Transform);
	ECS_TAG(ecs,World);
	ECS_TAG(ecs,Local);
	ecs_entity_t hero = ecs_entity(ecs, { .name = "Hero" });
	ecs_entity_t weapon = ecs_entity(ecs, { .name = "Weapon" });

	ecs_set_pair(ecs,hero,Transform, World, {GLM_MAT4_IDENTITY_INIT});
	ecs_set_pair(ecs,hero,Transform, Local, {GLM_MAT4_IDENTITY_INIT});
    
	ecs_set_pair(ecs,weapon,Transform, World, {GLM_MAT4_IDENTITY_INIT});
	ecs_set_pair(ecs,weapon,Transform, Local, {GLM_MAT4_IDENTITY_INIT});

    ecs_add_pair(ecs,weapon, EcsChildOf,hero);

      auto p = ecs_get_mut_pair(ecs, hero, Transform, Local);
      glm_translate(p->matrix, (vec3) {0,10,0});

     ecs_query_t *q = ecs_query(ecs, {
        .terms = {
            // Read from entity's Local position
            { .id = ecs_pair(ecs_id(Transform), Local), .inout = EcsIn }, 
            // Write to entity's World position
            { .id = ecs_pair(ecs_id(Transform), World), .inout = EcsOut },

            // Read from parent's World position
            {
                .id = ecs_pair(ecs_id(Transform), World), 
                .inout = EcsIn,
                // Get from the parent in breadth-first order (cascade)
                .src.id = EcsCascade,
                // Make parent term optional so we also match the root (sun)
                .oper = EcsOptional
            }
        }
    });

    ecs_iter_t it = ecs_query_iter(ecs, q);
    while (ecs_query_next(&it)) {
        Transform *p = ecs_field(&it, Transform, 0);
        Transform *p_out = ecs_field(&it, Transform, 1);
        Transform *p_parent = ecs_field(&it, Transform, 2);
        
        // Inner loop, iterates entities in archetype
        for (int i = 0; i < it.count; i ++) {
            glm_mat4_copy(p->matrix, p_out->matrix);
            if (p_parent) {
                glm_mat4_mul(p_parent->matrix, p_out->matrix, p_out->matrix);
            }
        }
    }
   it = ecs_each_pair_t(ecs, Transform, World);
    while (ecs_each_next(&it)) {
        Transform *p = ecs_field(&it, Transform, 0);
        for (int i = 0; i < it.count; i ++) {
            printf("%s: \n", ecs_get_name(ecs, it.entities[i]));
            glmc_mat4_print(p[i].matrix, stdout);
        }
    }

    ecs_fini(ecs);
}

MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    MU_RUN_TEST(test_check);
}

int
main(int argc, char* argv[]) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}