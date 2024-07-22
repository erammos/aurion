#pragma once
#include <stdlib.h>
char* read_file(const char* filename, size_t* size);
unsigned char* assets_load_image(const char* path, int* width, int* height);
void assets_free_image(unsigned char * data);