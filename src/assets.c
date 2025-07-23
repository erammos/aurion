#include "assets.h"
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define FAST_OBJ_IMPLEMENTATION
#include <graphics.h>
#include "fast_obj.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

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

c_mesh
assets_load_obj(const char* path,c_texture* texture) {
    fastObjMesh* fast_mesh = fast_obj_read(path);
    hashmap* hash = NULL;
    c_mesh mesh;


    mesh.num_v = 0;
    mesh.num_i = 0;
    mesh.indices = NULL;
    mesh.vertices = NULL;

    auto p = fast_mesh->textures[1];
    *texture = graphics_load_texture(p.path);

    for (int gi = 0; gi < fast_mesh->group_count; gi++) {
        fastObjGroup* grp = &fast_mesh->groups[gi];
        for (int i = 0; i < grp->face_count; i++) {
            int fv = fast_mesh->face_vertices[grp->face_offset + i];

            for (int j = 0; j < fv; j++) {
                g_vertex vertex;

                fastObjIndex mi = fast_mesh->indices[grp->index_offset + mesh.num_i];
                key_t key = {.p = mi.p, .n = mi.n, .t = mi.t};
                auto vv = hmget(hash, key);
                if (vv.index > 0) {
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
                arrput(mesh.vertices, vertex);
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

c_mesh assets_load_gltf(const char* path,c_texture* texture) {
    c_mesh mesh = {};
    cgltf_options options = {0};
    cgltf_data* data = NULL;

    // 1. Parse the file structure
    cgltf_result result = cgltf_parse_file(&options, path, &data);
    if (result != cgltf_result_success) {
        fprintf(stderr, "ERROR: Failed to parse GLTF file: %s\n", path);
        return mesh;
    }

    // 2. Load the binary data from .bin files or data URIs
    result = cgltf_load_buffers(&options, data, path);
    if (result != cgltf_result_success) {
        fprintf(stderr, "ERROR: Failed to load GLTF buffers for: %s\n", path);
        cgltf_free(data);
        return mesh;
    }

    // --- Now you can safely process the loaded data ---

    cgltf_mesh* gltf_mesh = &data->meshes[0];
    cgltf_primitive* primitive = &gltf_mesh->primitives[0];

    // --- Vertex Data Loading (Restored) ---
    mesh.num_v = primitive->attributes[0].data->count;
    mesh.vertices = (g_vertex*)malloc(mesh.num_v * sizeof(g_vertex));
    for (size_t i = 0; i < mesh.num_v; ++i) {
        // Find and copy position data
        cgltf_accessor* pos_accessor = NULL;
        for (size_t j = 0; j < primitive->attributes_count; ++j) {
            if (primitive->attributes[j].type == cgltf_attribute_type_position) {
                pos_accessor = primitive->attributes[j].data;
                break;
            }
        }
        if (pos_accessor) {
            cgltf_accessor_read_float(pos_accessor, i, mesh.vertices[i].position, 3);
        }

        // Find and copy normal data
        cgltf_accessor* normal_accessor = NULL;
        for (size_t j = 0; j < primitive->attributes_count; ++j) {
            if (primitive->attributes[j].type == cgltf_attribute_type_normal) {
                normal_accessor = primitive->attributes[j].data;
                break;
            }
        }
        if (normal_accessor) {
            cgltf_accessor_read_float(normal_accessor, i, mesh.vertices[i].normal, 3);
        }

        // Find and copy texture coordinate data
        cgltf_accessor* tex_coord_accessor = NULL;
        for (size_t j = 0; j < primitive->attributes_count; ++j) {
            if (primitive->attributes[j].type == cgltf_attribute_type_texcoord) {
                tex_coord_accessor = primitive->attributes[j].data;
                break;
            }
        }
        if (tex_coord_accessor) {
            cgltf_accessor_read_float(tex_coord_accessor, i, mesh.vertices[i].uv, 2);
        }
    }

    // --- Index Data Loading (with fixes) ---
    if (primitive->indices != NULL) {
        mesh.num_i = primitive->indices->count;
        mesh.indices = (unsigned int*)malloc(mesh.num_i * sizeof(unsigned int));
        for (size_t i = 0; i < mesh.num_i; ++i) {
            mesh.indices[i] = (unsigned int)cgltf_accessor_read_index(primitive->indices, i);
        }
    } else {
        mesh.num_i = 0;
        mesh.indices = NULL;
    }

    // --- Texture Loading Logic ---
    if (primitive->material && primitive->material->has_pbr_metallic_roughness) {
        cgltf_texture_view* tex_view = &primitive->material->pbr_metallic_roughness.base_color_texture;
        if (tex_view->texture && tex_view->texture->image) {
            const char* image_uri = tex_view->texture->image->uri;
            if (image_uri && texture != nullptr) {
                char full_path[1024];
                snprintf(full_path, sizeof(full_path), "assets/%s", image_uri);
                *texture = graphics_load_texture(full_path);
            }
        }
    }

    // ... (Your OpenGL VAO/VBO/EBO generation code would go here) ...

    cgltf_free(data);
    return mesh;
}
