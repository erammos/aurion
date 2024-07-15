#include "assets.h"
#include <stdio.h>
#include "freetype/freetype.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
FT_Library ft;

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
    buffer = malloc(*size * sizeof(char));
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

    fclose(file); // Close the file
    return buffer;
}

void
assets_init() {
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "ERROR::FREETYPE: Could not init FreeType Library");
        exit(1);
    }

    stbi_set_flip_vertically_on_load(true);
}

void
assets_load_font(const char* font) {}

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
