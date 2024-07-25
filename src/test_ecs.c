#include <cglm/cglm.h>
#include "minunit.h"
#include <stdio.h>
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

int idx = 0;
for(int gi = 0 ; gi < mesh->group_count; gi++)
{
 fastObjGroup * grp = &mesh->groups[gi];
 for(int i = 0 ; i < grp->face_count ; i++)
 {
    int fv = mesh->face_vertices[grp->face_offset + i];

    for(int j = 0 ; j < fv ; j++)
    {
    
        fastObjIndex mi = mesh->indices[grp->index_offset + idx];
         if(mi.p)
         {
            float x = mesh->positions[3 * mi.p + 0];
            float y = mesh->positions[3 * mi.p + 1];
            float z = mesh->positions[3 * mi.p + 2];
            printf ("pos: %f %f %f\n",x,y,z);
         }
          if (mi.t)
                {
                float u =  mesh->texcoords[2 * mi.t + 0];
                float v = mesh->texcoords[2 * mi.t + 1];
                printf ("uv: %f %f\n",u,v);
                }
           if(mi.n)
           {
              float nx = mesh->normals[3 * mi.n + 0];
              float ny = mesh->normals[3 * mi.n + 1];
              float nz = mesh->normals[3 * mi.n + 2];
            printf ("normal: %f %f %f\n",nx,ny,nz);
           }
         idx++;
    }
 }
}
printf("indices:\n");
    for(int ii = 0 ; ii < mesh->index_count; ii++)
    {
       auto p = mesh->indices[ii];
    
       printf("%d \n", p.p);
    }
 fast_obj_destroy(mesh);

   
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