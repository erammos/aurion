#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    // Convert to world space
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.FragPos = worldPos.xyz;

    // Transform normal correctly (assumes no non-uniform scaling)
    vs_out.Normal = mat3(model) * aNormal;

    vs_out.TexCoords = aTexCoords;

    // Final position in clip space
    gl_Position = projection * view * worldPos;
}
