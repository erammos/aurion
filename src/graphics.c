#include "graphics.h"
#include <SDL2/SDL.h>
#include <assets.h>
#include <glad/glad.h>
#include "cglm/cam.h"
#include "cglm/vec3.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#define DEBUG

void
checkOpenGLError(const char* stmt, const char* fname, int line) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
        exit(1);
    }
}

#define GL_CHECK(stmt)                                                                                                 \
    {                                                                                                                  \
        stmt;                                                                                                          \
        checkOpenGLError(#stmt, __FILE__, __LINE__);                                                                   \
    }

static void* g_window = {};
static void* g_context = {};
static int screen_width = {};
static int screen_height = {};
static g_shader active_shader;
static g_camera active_camera;

double
degrees(double radians) {
    return radians * (180.0 / M_PI);
}

double
radians(double degrees) {
    return degrees * (M_PI / 180.0);
}

void APIENTRY
glDebugOutputARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                 const void* userParam) {
    // Ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
        return;
    }

    printf("---------------\n");
    printf("Debug message (%d): %s\n", id, message);

    switch (source) {
        case GL_DEBUG_SOURCE_API: printf("Source: API\n"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: printf("Source: Window System\n"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: printf("Source: Shader Compiler\n"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: printf("Source: Third Party\n"); break;
        case GL_DEBUG_SOURCE_APPLICATION: printf("Source: Application\n"); break;
        case GL_DEBUG_SOURCE_OTHER: printf("Source: Other\n"); break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR: printf("Type: Error\n"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: printf("Type: Deprecated Behaviour\n"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: printf("Type: Undefined Behaviour\n"); break;
        case GL_DEBUG_TYPE_PORTABILITY: printf("Type: Portability\n"); break;
        case GL_DEBUG_TYPE_PERFORMANCE: printf("Type: Performance\n"); break;
        case GL_DEBUG_TYPE_MARKER: printf("Type: Marker\n"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP: printf("Type: Push Group\n"); break;
        case GL_DEBUG_TYPE_POP_GROUP: printf("Type: Pop Group\n"); break;
        case GL_DEBUG_TYPE_OTHER: printf("Type: Other\n"); break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: printf("Severity: high\n"); break;
        case GL_DEBUG_SEVERITY_MEDIUM: printf("Severity: medium\n"); break;
        case GL_DEBUG_SEVERITY_LOW: printf("Severity: low\n"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: printf("Severity: notification\n"); break;
    }
    printf("\n");
}

int
graphics_init(void* window) {
    g_window = window;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    g_context = SDL_GL_CreateContext(window);

    if (g_context == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create OpenGL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_GL_MakeCurrent(window, g_context);
    SDL_GL_GetDrawableSize(window, &screen_width, &screen_height);
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD");
        return -1;
    }
    if (GLAD_GL_ARB_debug_output) {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallbackARB(glDebugOutputARB, NULL);
        glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    } else {
        fprintf(stderr, "GL_ARB_debug_output not supported\n");
    }
    return 0;
}

static bool
load_shader(GLuint* uiShader, GLenum ShaderType, const GLchar* p_cShader) {
    // Build and link the shader program
    *uiShader = glCreateShader(ShaderType);
    glShaderSource(*uiShader, 1, &p_cShader, NULL);
    glCompileShader(*uiShader);

    // Check for errors
    GLint iTestReturn;
    glGetShaderiv(*uiShader, GL_COMPILE_STATUS, &iTestReturn);
    if (iTestReturn == GL_FALSE) {
        GLchar p_cInfoLog[1024];
        int32_t iErrorLength;
        glGetShaderInfoLog(*uiShader, 1024, &iErrorLength, p_cInfoLog);
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile shader: %s\n", p_cInfoLog);
        glDeleteShader(*uiShader);
        return false;
    }
    return true;
}

void
graphics_set_transform(mat4 transform) {
    GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(active_shader.id, "model"), 1, GL_FALSE, &transform[0][0]));
}

int
graphics_load_shaders(g_shader* shader, const char* vs_file, const char* fs_file) {
    size_t vs_size, fs_size = 0;
    auto vertex_shader = read_file(vs_file, &vs_size);
    auto fragment_shader = read_file(fs_file, &fs_size);

    GLuint vs_id = 0;
    load_shader(&vs_id, GL_VERTEX_SHADER, vertex_shader);
    GLuint fs_id = 0;
    load_shader(&fs_id, GL_FRAGMENT_SHADER, fragment_shader);
    auto program = glCreateProgram();
    glAttachShader(program, vs_id);
    glAttachShader(program, fs_id);
    glLinkProgram(program);
    GLint iTestReturn;
    glGetProgramiv(program, GL_LINK_STATUS, &iTestReturn);
    if (iTestReturn == GL_FALSE) {
        GLchar p_cInfoLog[1024];
        int32_t iErrorLength;
        glGetProgramInfoLog(program, 1024, &iErrorLength, p_cInfoLog);
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to link shaders: %s\n", p_cInfoLog);
        glDeleteProgram(program);
        return false;
    }
    shader->id = program;
    return true;
}

void
graphics_destroy() {
    SDL_GL_DeleteContext(g_context);
}

g_mesh
graphics_create_mesh(size_t num_vertices, size_t num_indices, size_t num_textures,
                     g_vertex vertices[static num_vertices], unsigned int indices[static num_indices],
                     g_texture textures[static num_textures]) {
    g_mesh mesh = {

        .num_i = num_indices, .num_v = num_vertices, .num_t = num_textures};

    mesh.vertices = calloc(num_vertices, sizeof(g_vertex));
    mesh.indices = calloc(num_indices, sizeof(unsigned int));
    mesh.textures = calloc(num_textures, sizeof(g_texture));

    memcpy(mesh.vertices, vertices, sizeof(g_vertex) * num_vertices);
    memcpy(mesh.indices, indices, sizeof(unsigned int) * num_indices);
    memcpy(mesh.textures, textures, sizeof(g_texture) * num_textures);

    GL_CHECK(glGenVertexArrays(1, &mesh.vao));
    GL_CHECK(glGenBuffers(1, &mesh.vbo));
    GL_CHECK(glGenBuffers(1, &mesh.ebo));

    GL_CHECK(glBindVertexArray(mesh.vao));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo));

    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, mesh.num_v * sizeof(g_vertex), &mesh.vertices[0], GL_STATIC_DRAW));

    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo));
    GL_CHECK(
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.num_i * sizeof(unsigned int), &mesh.indices[0], GL_STATIC_DRAW));

    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(g_vertex), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(g_vertex), (void*)offsetof(g_vertex, normal)));
    GL_CHECK(glEnableVertexAttribArray(2));
    GL_CHECK(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(g_vertex), (void*)offsetof(g_vertex, uv)));

    GL_CHECK(glBindVertexArray(0));
    return mesh;
}

void
graphics_draw_mesh(g_mesh* mesh) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    for (unsigned int i = 0; i < mesh->num_t; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        char number[2] = {[1] = '\0'};
        char name[30] = {};
        strcpy(name, mesh->textures[i].type);
        if (strcmp(name, "texture_diffuse") == 0) {
            number[0] = '0' + diffuseNr++;
        } else if (strcmp(name, "texture_specular") == 0) {
            number[0] = '0' + specularNr++;
        }

        strcat(name, number);
        GL_CHECK(glUniform1i(glGetUniformLocation(active_shader.id, name), i));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id));
    }
    GL_CHECK(glActiveTexture(GL_TEXTURE0));

    // draw mesh
    GL_CHECK(glBindVertexArray(mesh->vao));
    GL_CHECK(glDrawElements(GL_TRIANGLES, mesh->num_i, GL_UNSIGNED_INT, 0));
    GL_CHECK(glBindVertexArray(0));
}

void
graphics_begin() {
    glClearColor(1, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

void
graphics_use_shader(g_shader* shader) {
    active_shader = *shader;
    glUseProgram(shader->id);
}

void
graphics_end() {
    SDL_GL_SwapWindow(g_window);
}

void
graphics_set_camera(vec3 pos, vec3 up, float yaw, float pitch) {
    mat4 view;
    active_camera.front[0] = cos(radians(yaw)) * cos(radians(pitch));
    active_camera.front[1] = sin(radians(pitch));
    active_camera.front[2] = sin(radians(yaw)) * cos(radians(pitch));

    glm_vec3_copy(pos, active_camera.position);
    glm_normalize(active_camera.front);
    glm_cross(active_camera.front, up, active_camera.right);
    glm_normalize(active_camera.right);
    glm_cross(active_camera.right, active_camera.front, active_camera.up);
    glm_normalize(active_camera.up);

    glm_vec3_add(active_camera.position, active_camera.front, active_camera.target);
    glm_lookat(active_camera.position, active_camera.target, active_camera.up, view);

    GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(active_shader.id, "view"), 1, GL_FALSE, &view[0][0]));
}

g_camera
graphics_get_active_camera() {
    return active_camera;
}

void
graphics_camera_perspective() {
    mat4 projection;
    glm_perspective(radians(45), screen_width / (float)screen_height, 0.1f, 100.0f, projection);
    GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(active_shader.id, "projection"), 1, GL_FALSE, &projection[0][0]));
}

void
graphics_camera_ortho() {
    mat4 orthographic;
    glm_ortho(-1, 1, -1, 1, 0.1f, 100.0f, orthographic);
    GL_CHECK(
        glUniformMatrix4fv(glGetUniformLocation(active_shader.id, "projection"), 1, GL_FALSE, &orthographic[0][0]));
}

g_texture
graphics_load_texture(const char* path) {
    int width, height;
    int nrchannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    unsigned char* data = stbi_load(path, &width, &height, &nrchannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        // glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        /* code here */
        printf(stderr, "Failed to load image");
        exit(1);
    }
    stbi_image_free(data);
    return (g_texture){.id = texture, .type = "texture_diffuse"};
}