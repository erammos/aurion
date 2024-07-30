#version 330 core
out vec4 FragColor;


uniform vec3 orbColor;
uniform float intensity;

void main() {
    // Simple emissive effect
    vec3 emissive = orbColor * intensity;
    FragColor = vec4(vec3(1,1,1), 1.0);
}