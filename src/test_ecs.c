#include <cglm/cglm.h>
#include <stdio.h>
#include "graphics.h"
#include "minunit.h"
#define FAST_OBJ_IMPLEMENTATION

#include "fast_obj.h"

void
test_setup(void) {}

void
test_teardown(void) {
    /* Nothing */
}

MU_TEST(test_check) {
    fastObjMesh* mesh = fast_obj_read("assets/test.obj");

    g_mesh gmesh;

    gmesh.num_v = mesh->face_count * 3;
    gmesh.num_i = gmesh.num_v;
    gmesh.num_t = mesh->texture_count;
    gmesh.indices = malloc(sizeof(unsigned int) * gmesh.num_i);
    gmesh.vertices = malloc(sizeof(g_vertex) * gmesh.num_v);

    int idx = 0;
    for (int gi = 0; gi < mesh->group_count; gi++) {
        fastObjGroup* grp = &mesh->groups[gi];
        for (int i = 0; i < grp->face_count; i++) {
            int fv = mesh->face_vertices[grp->face_offset + i];

            for (int j = 0; j < fv; j++) {
                g_vertex vertex;
                fastObjIndex mi = mesh->indices[grp->index_offset + idx];
                gmesh.indices[idx] = idx;
                printf("index: %d\n", idx);
                if (mi.p) {
                    vertex.position[0] = mesh->positions[3 * mi.p + 0];
                    vertex.position[1] = mesh->positions[3 * mi.p + 1];
                    vertex.position[2] = mesh->positions[3 * mi.p + 2];
                }
                if (mi.t) {
                    vertex.uv[0] = mesh->texcoords[2 * mi.t + 0];
                    vertex.uv[1] = mesh->texcoords[2 * mi.t + 1];
                }
                if (mi.n) {
                    vertex.normal[0] = mesh->normals[3 * mi.n + 0];
                    vertex.normal[1] = mesh->normals[3 * mi.n + 1];
                    vertex.normal[2] = mesh->normals[3 * mi.n + 2];
                }
                idx++;
            }
        }
    }
}

// g_vertex vertex;
// unsigned int indices[mesh->index_count];
// for(int i = 0; i < mesh->position_count; i++)
// {
//     vertex.position[0] = mesh->positions[i];
//     vertex.position[1] = mesh->positions[i];
//     vertex.position[2] = mesh->positions[i];
//     printf ("pos: %f %f %f\n",vertex.position[0], vertex.position[1], vertex.position[2]);
// }

// printf("indices: %d\n",mesh->index_count);
//     for(int ii = 0 ; ii < mesh->index_count; ii++)
//     {

//        auto obj_index = mesh->indices[ii];
//         indices[ii] = obj_index.p - 1;
//        printf("index: %d \n", indices[ii]);
//     }
//    fast_obj_destroy(mesh);
// }

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