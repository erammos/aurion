#pragma once
#include <stdlib.h>
#include <graphics.h>
#include <components.h>
char* read_file(const char* filename, size_t* size);
unsigned char* assets_load_image(const char* path, int* width, int* height);
void assets_free_image(unsigned char * data);
c_mesh assets_load_obj(const char * path,c_texture * texture);
c_mesh assets_load_gltf(const char * path,c_texture * texture);