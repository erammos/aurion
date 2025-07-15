#pragma once
#include <stdlib.h>
#include <graphics.h>
char* read_file(const char* filename, size_t* size);
unsigned char* assets_load_image(const char* path, int* width, int* height);
void assets_free_image(unsigned char * data);
g_mesh assets_load_obj(const char * path,g_material * material);
g_mesh assets_load_gltf(const char * path,g_material * material);