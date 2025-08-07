#include "graphics.h"
#include <SDL2/SDL.h>
#include <assets.h>
#include <glad/glad.h>
#include "assets.h"
#include "cglm/cam.h"
#include "cglm/vec3.h"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

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
static g_camera* active_camera;

int
graphics_get_width() {
    return screen_width;
}

int
graphics_get_height() {
    return screen_height;
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
    SDL_GetWindowSize(window, &screen_width, &screen_height);

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

static void
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
        exit(1);
    }
}

void
graphics_set_transform(mat4 transform) {
    graphics_set_uniform_mat4("model", transform);
}

g_shader
graphics_load_shaders(const char* vs_file, const char* fs_file) {

    g_shader shader;
    size_t vs_size, fs_size = 0;
    auto vertex_shader = read_file(vs_file, &vs_size);
    auto fragment_shader = read_file(fs_file, &fs_size);
    printf("%s\n\n", vertex_shader);
    printf("%s\n", fragment_shader);
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
        exit(1);
    }
    shader.id = program;
    return shader;
}

void
graphics_destroy() {
    SDL_GL_DeleteContext(g_context);
}

void
graphics_create_gl_buffer(c_mesh* mesh) {
    GL_CHECK(glGenVertexArrays(1, &mesh->vao));
    GL_CHECK(glGenBuffers(1, &mesh->vbo));
    GL_CHECK(glGenBuffers(1, &mesh->ebo));

    GL_CHECK(glBindVertexArray(mesh->vao));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo));

    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, mesh->num_v * sizeof(g_vertex), &mesh->vertices[0], GL_STATIC_DRAW));

    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo));
    GL_CHECK(
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->num_i * sizeof(unsigned int), &mesh->indices[0], GL_STATIC_DRAW));

    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(g_vertex), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(g_vertex), (void*)offsetof(g_vertex, normal)));
    GL_CHECK(glEnableVertexAttribArray(2));
    GL_CHECK(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(g_vertex), (void*)offsetof(g_vertex, uv)));

    GL_CHECK(glBindVertexArray(0));
}

c_mesh
graphics_create_mesh(size_t num_vertices, size_t num_indices,
                     g_vertex vertices[static num_vertices], unsigned int indices[static num_indices]
                     ) {
    c_mesh mesh = {

        .num_i = num_indices, .num_v = num_vertices};

    mesh.vertices = calloc(num_vertices, sizeof(g_vertex));
    memcpy(mesh.vertices, vertices, sizeof(g_vertex) * num_vertices);
    mesh.indices = calloc(num_indices, sizeof(unsigned int));
    memcpy(mesh.indices, indices, sizeof(unsigned int) * num_indices);

    graphics_create_gl_buffer(&mesh);

    return mesh/**/;
}

void graphics_bind_texture(c_texture tex) {

        glActiveTexture(GL_TEXTURE0);
        graphics_set_uniform_int(tex.type, 0);
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex.id));
        GL_CHECK(glActiveTexture(GL_TEXTURE0));
}

void
graphics_draw_mesh(c_mesh* mesh) {


    // draw mesh
    GL_CHECK(glBindVertexArray(mesh->vao));
    GL_CHECK(glDrawElements(GL_TRIANGLES, mesh->num_i, GL_UNSIGNED_INT, 0));
    GL_CHECK(glBindVertexArray(0));
}

void
graphics_begin() {
    SDL_GetWindowSize(g_window, &screen_width, &screen_height);
    glViewport(0, 0, screen_width, screen_height);
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
graphics_use_camera(g_camera* camera) {
    active_camera = camera;
    graphics_set_uniform_mat4("view", camera->view);
    graphics_set_uniform_mat4("projection", camera->projection);
}

g_camera*
graphics_get_active_camera() {
    return active_camera;
}

c_texture
graphics_load_texture(const char* path) {
    int width, height;
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    unsigned char* data = assets_load_image(path, &width, &height);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        /* code here */
        fprintf(stderr, "Failed to load image");
        exit(1);
    }
    assets_free_image(data);
    return (c_texture){.id = texture, .type = "texture_diffuse"};
}

unsigned int graphics_load_cubemap(const char** faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < 6; i++) {
        unsigned char *data = assets_load_image(faces[i], &width, &height);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            assets_free_image(data);
        } else {
            assets_free_image(data);
            printf("Cubemap texture failed to load at path: %s\n", faces[i]);
            exit(1);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

c_mesh graphics_create_skybox_mesh() {
    float skyboxVertices[] = {
        // positions
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };

    c_mesh mesh = {0};
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    mesh.num_v = 36; // 6 faces * 2 triangles * 3 vertices
    return mesh;
}

c_mesh
graphics_load_model(const char* path, c_texture * texture) {
    const char* dot = strrchr(path, '.');

    c_mesh mesh;
    // Ensure a dot was found and it's not the first character (e.g., ".bashrc")
    if (dot && dot != path) {
        // Compare the substring after the dot with our known extensions
        if (strcmp(dot, ".obj") == 0) {
            mesh = assets_load_obj(path, texture);
        }
        if (strcmp(dot, ".gltf") == 0) {
            mesh = assets_load_gltf(path, texture);
        }
    }
    graphics_create_gl_buffer(&mesh);
    return mesh;
}


void
graphics_set_light(vec3 pos, vec3 viewPos, vec3 lightColor) {
    graphics_set_uniform_vec3("lightPos", pos);
    graphics_set_uniform_vec3("viewPos", viewPos);
    graphics_set_uniform_vec3("lightColor",lightColor);
}

static float
perlin_noise(float x, float y, int octaves, float lacunarity, float persistence) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f; // Used for normalizing result to the range [0,1]

    for (int i = 0; i < octaves; i++) {
        total += stb_perlin_noise3(x * frequency, y * frequency, 0, 0, 0, 0) * amplitude;

        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total / maxValue;
}
static inline float smoothstep(float edge0, float edge1, float x) {
    // Scale, bias and saturate x to 0..1 range
    x = (x - edge0) / (edge1 - edge0);
    x = x > 1.0f ? 1.0f : (x < 0.0f ? 0.0f : x);
    // Evaluate polynomial
    return x * x * (3.0f - 2.0f * x);
}

c_mesh graphics_create_terrain(int terrain_width, int terrain_height) {

    g_vertex * vertices = calloc(terrain_width * terrain_height,sizeof(g_vertex));
    unsigned int * indices = calloc( (terrain_width - 1) * (terrain_height - 1) * 6, sizeof(unsigned int));

    // --- Arena Generation Parameters (Tweak these to change the arena shape) ---

    // Defines the radius of the flat central playable area.
    const float center_radius = (float)terrain_width * 0.2f;
    // Defines the radius where the mountains reach their full height.
    const float mountain_radius = (float)terrain_width * 0.55f;
    // The maximum height of the mountains at the border.
    const float mountain_height = 40.0f;
    // How rugged the mountains are. Higher values mean more jagged peaks.
    const float mountain_ruggedness = 20.0f;
    // Controls the scale/zoom of the noise pattern for the entire map.
    const float perlin_scale = 0.05f;

    // --- Vertex Generation ---
    const float center_x = (float)terrain_width / 2.0f;
    const float center_y = (float)terrain_height / 2.0f;

    for (int y = 0; y < terrain_height; ++y) {
        for (int x = 0; x < terrain_width; ++x) {
            // Calculate the vertex's distance from the center of the map.
            const float dist_x = (float)x - center_x;
            const float dist_y = (float)y - center_y;
            const float dist_from_center = sqrtf(dist_x * dist_x + dist_y * dist_y);

            // Use smoothstep to create a weight that is 0 in the center and 1 at the edges.
            // This creates a smooth transition from the flat ground to the mountains.
            const float mountain_weight = smoothstep(center_radius, mountain_radius, dist_from_center);

            // Generate the base Perlin noise.
            float noise = perlin_noise((float)x * perlin_scale, (float)y * perlin_scale, 6, 2.0f, 0.5f);

            // Calculate the final height.
            // 1. The base height is determined by the "bowl" shape (mountain_weight * mountain_height).
            // 2. The noise is scaled by the ruggedness and the mountain_weight, making the center smoother
            //    and the edges more chaotic.
            const float final_height = (mountain_weight * mountain_height) + (noise * mountain_ruggedness * mountain_weight);

            // Set vertex position and UV coordinates.
            const int current_vertex = y * terrain_width + x;
            glm_vec3_copy((vec3){(float)x, final_height, (float)y}, vertices[current_vertex].position);
            glm_vec2_copy((vec2){(float)x / terrain_width, (float)y / terrain_height}, vertices[current_vertex].uv);
        }
    }

    // --- Normal Calculation (Unchanged) ---
    // This part of the logic remains the same, as it correctly calculates normals
    // based on the final vertex positions.
    for (int y = 0; y < terrain_height; ++y) {
        for (int x = 0; x < terrain_width; ++x) {
            vec3 normal = {0.0f, 1.0f, 0.0f}; // Default normal pointing up

            if (x > 0 && x < terrain_width - 1 && y > 0 && y < terrain_height - 1) {
                vec3 left, right, down, up;
                glm_vec3_copy(vertices[y * terrain_width + (x - 1)].position, left);
                glm_vec3_copy(vertices[y * terrain_width + (x + 1)].position, right);
                glm_vec3_copy(vertices[(y - 1) * terrain_width + x].position, down);
                glm_vec3_copy(vertices[(y + 1) * terrain_width + x].position, up);

                vec3 dx, dy;
                glm_vec3_sub(right, left, dx);
                glm_vec3_sub(up, down, dy);

                glm_vec3_cross(dy, dx, normal); // Flipped cross product for correct normal direction
                glm_vec3_normalize(normal);
            }

            glm_vec3_copy(normal, vertices[y * terrain_width + x].normal);
        }
    }

    // --- Index Generation (Unchanged) ---
    int idx = 0;
    for (int y = 0; y < terrain_height - 1; ++y) {
        for (int x = 0; x < terrain_width - 1; ++x) {
            int topLeft = y * terrain_width + x;
            int topRight = topLeft + 1;
            int bottomLeft = (y + 1) * terrain_width + x;
            int bottomRight = bottomLeft + 1;

            indices[idx++] = topLeft;
            indices[idx++] = bottomLeft;
            indices[idx++] = topRight;

            indices[idx++] = topRight;
            indices[idx++] = bottomLeft;
            indices[idx++] = bottomRight;
        }
    }

    c_mesh mesh = {};
    mesh.num_v = terrain_width * terrain_height;
    mesh.vertices = vertices;
    mesh.indices = indices;
    mesh.num_i = (terrain_width - 1) * (terrain_height - 1) * 6;
    graphics_create_gl_buffer(&mesh);
    return mesh;
}

static void
generate_circle(vec3* circle, int segments, float radius) {
    float angleStep = 2.0f * M_PI / segments;
    for (int i = 0; i < segments; ++i) {
        float angle = i * angleStep;
        circle[i][0] = cosf(angle) * radius;
        circle[i][1] = sinf(angle) * radius;
        circle[i][2] = 0.0f; // Circle lies in the XY plane
    }
}


c_mesh
graphics_generate_tunnel(int segments, int length, float radius) {
    vec3* circle = (vec3*)malloc(segments * sizeof(vec3));
    generate_circle(circle, segments, radius);

    c_mesh mesh;
    mesh.num_v = segments * length;
    mesh.num_i = segments * (length - 1) * 6;
    mesh.vertices = (g_vertex*)malloc(mesh.num_v * sizeof(g_vertex));
    mesh.indices = (unsigned int*)malloc(mesh.num_i * sizeof(unsigned int));

    int vertexCount = 0;
    int indexCount = 0;

    for (int i = 0; i < length; ++i) {
        for (int j = 0; j < segments; ++j) {
            vec3 position;
            glm_vec3_copy(circle[j], position);
            position[2] = (float)i; // Move circle along Z axis

            glm_vec3_copy(position, mesh.vertices[vertexCount].position);
            glm_vec3_normalize_to(position, mesh.vertices[vertexCount].normal);
            mesh.vertices[vertexCount].uv[0] = (float)j / segments;
            mesh.vertices[vertexCount].uv[1] = (float)i / length;

            if (i < length - 1) {
                int nextSegment = (j + 1) % segments;
                int currentIndex = i * segments + j;
                int nextIndex = currentIndex + segments;
                int nextSegmentIndex = i * segments + nextSegment;
                int nextSegmentNextIndex = nextSegmentIndex + segments;

                // First triangle
                mesh.indices[indexCount++] = currentIndex;
                mesh.indices[indexCount++] = nextIndex;
                mesh.indices[indexCount++] = nextSegmentNextIndex;

                // Second triangle
                mesh.indices[indexCount++] = currentIndex;
                mesh.indices[indexCount++] = nextSegmentNextIndex;
                mesh.indices[indexCount++] = nextSegmentIndex;
            }
            vertexCount++;
        }
    }

    free(circle);
    graphics_create_gl_buffer(&mesh);
    return mesh;
}

c_mesh
create_orb_mesh(float radius, int sectors, int stacks) {
    int numVertices = (sectors + 1) * (stacks + 1);
    int numIndices = sectors * stacks * 6;
    g_vertex* vertices = malloc(sizeof(g_vertex) * numVertices);
    unsigned int* indices = malloc(sizeof(unsigned int) * numIndices);

    float x, y, z, xy;                           // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius; // normal
    float s, t;                                  // texCoord

    float sectorStep = 2 * M_PI / sectors;
    float stackStep = M_PI / stacks;
    float sectorAngle, stackAngle;

    int k = 0;
    for (int i = 0; i <= stacks; ++i) {
        stackAngle = M_PI / 2 - i * stackStep; // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);        // r * cos(u)
        z = radius * sinf(stackAngle);         // r * sin(u)

        for (int j = 0; j <= sectors; ++j, ++k) {
            sectorAngle = j * sectorStep; // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)
            glm_vec3_copy((vec3){x, y, z}, vertices[k].position);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            glm_vec3_copy((vec3){nx, ny, nz}, vertices[k].normal);

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectors;
            t = (float)i / stacks;
            glm_vec2_copy((vec2){s, t}, vertices[k].uv);
        }
    }

    int index = 0;
    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1); // beginning of current stack
        int k2 = k1 + sectors + 1;  // beginning of next stack

        for (int j = 0; j < sectors; ++j, ++index) {
            if (i != 0) {
                indices[index++] = k1 + j;
                indices[index++] = k2 + j;
                indices[index++] = k1 + j + 1;
            }

            if (i != (stacks - 1)) {
                indices[index++] = k1 + j + 1;
                indices[index++] = k2 + j;
                indices[index++] = k2 + j + 1;
            }
        }
    }

    c_mesh orbMesh = graphics_create_mesh(numVertices, numIndices, vertices, indices);

    free(vertices);
    free(indices);

    return orbMesh;
}

void
graphics_set_uniform_vec3(const char* name, vec3 vec) {
    GL_CHECK(glUniform3fv(glGetUniformLocation(active_shader.id, name), 1, &vec[0]));
}

void
graphics_set_uniform_float(const char* name, float value) {
    GL_CHECK(glUniform1f(glGetUniformLocation(active_shader.id, name), value));
}

void
graphics_set_uniform_mat4(const char* name, mat4 matrix) {
    GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(active_shader.id, name), 1, GL_FALSE, &matrix[0][0]));
}

void
graphics_set_uniform_int(const char* name, int value) {
    GL_CHECK(glUniform1i(glGetUniformLocation(active_shader.id, name), value));
}
