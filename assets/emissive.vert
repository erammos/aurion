#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragUV;
out mat4 outModel;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    outModel = model;
    fragPosition = vec3(model * vec4(aPos, 1.0));
    fragNormal = mat3(transpose(inverse(model))) * aNormal;
    fragUV = aTexCoords;
    gl_Position = projection * view * vec4(fragPosition, 1.0);
}
