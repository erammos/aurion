#include "assets.h"
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define FAST_OBJ_IMPLEMENTATION
#include <graphics.h>
#include "fast_obj.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

char*
read_file(const char* filename, size_t* size) {
    FILE* file = fopen(filename, "r+"); // Open file in binary mode
    char* buffer = nullptr;

    if (file == nullptr) {
        perror("Error opening file");
        exit(1);
    }

    // Seek to the end of the file to get the size
    fseek(file, 0, SEEK_END);
    *size = ftell(file); // Get the size of the file
    rewind(file);        // Go back to the beginning of the file

    // Allocate memory for the buffer
    buffer = malloc(*size * sizeof(char) + 1);
    if (buffer == nullptr) {
        perror("Error allocating memory");
        fclose(file);
        exit(1);
    }

    // Read the file into the buffer
    size_t read_size = fread(buffer, 1, *size, file);
    if (read_size != *size) {
        perror("Error reading file");
        free(buffer);
        fclose(file);
        exit(1);
    }

    buffer[*size] = '\0';

    fclose(file); // Close the file
    return buffer;
}

unsigned char*
assets_load_image(const char* path, int* width, int* height) {
    int nrchannels;
    unsigned char* data = stbi_load(path, width, height, &nrchannels, 0);
    return data;
}

void
assets_free_image(unsigned char* data) {
    stbi_image_free(data);
}

typedef struct {
    int p, t, n;
} key_t;
typedef struct {
    unsigned int index;
    g_vertex vertex;
} value_t;
typedef struct {
    key_t key;
    value_t value;
} hashmap;

g_mesh
assets_load_obj(const char* path) {
    fastObjMesh* fast_mesh = fast_obj_read(path);
    hashmap* hash = NULL;
    g_mesh mesh;

    mesh.num_v =  0;
    mesh.num_i = 0;
    mesh.num_t = fast_mesh->texture_count - 1;
    mesh.indices = NULL;
    mesh.vertices = NULL;
    mesh.textures = malloc(sizeof(g_texture) * mesh.num_t);

    auto p = fast_mesh->textures[1];
    g_texture texture = graphics_load_texture(p.path);
    mesh.textures[0] = texture;

    for (int gi = 0; gi < fast_mesh->group_count; gi++) {
        fastObjGroup* grp = &fast_mesh->groups[gi];
        for (int i = 0; i < grp->face_count; i++) {
            int fv = fast_mesh->face_vertices[grp->face_offset + i];

            for (int j = 0; j < fv; j++) {
                g_vertex vertex;

                fastObjIndex mi = fast_mesh->indices[grp->index_offset + mesh.num_i];
                key_t key = {.p = mi.p, .n = mi.n, .t = mi.t};
                auto vv = hmget(hash,key);
                if(vv.index  > 0)
                {
                    arrput(mesh.indices, vv.index - 1);
                    mesh.num_i++;
                    continue;
                }
                if (mi.p) {
                    vertex.position[0] = fast_mesh->positions[3 * mi.p + 0];
                    vertex.position[1] = fast_mesh->positions[3 * mi.p + 1];
                    vertex.position[2] = fast_mesh->positions[3 * mi.p + 2];
                }
                if (mi.t) {
                    vertex.uv[0] = fast_mesh->texcoords[2 * mi.t + 0];
                    vertex.uv[1] = fast_mesh->texcoords[2 * mi.t + 1];
                }
                if (mi.n) {
                    vertex.normal[0] = fast_mesh->normals[3 * mi.n + 0];
                    vertex.normal[1] = fast_mesh->normals[3 * mi.n + 1];
                    vertex.normal[2] = fast_mesh->normals[3 * mi.n + 2];
                }
                arrput(mesh.indices, mesh.num_v);
                arrput(mesh.vertices,vertex);
                value_t value = {mesh.num_v + 1, vertex};
                hmput(hash, key, value);
                mesh.num_v++;
                mesh.num_i++;
            }
        }
    }
    hmfree(hash);
    fast_obj_destroy(fast_mesh);
    return mesh;
}
