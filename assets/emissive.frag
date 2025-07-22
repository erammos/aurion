#version 330 core
out vec4 FragColor;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragUV;
in mat4 outModel;

uniform vec3 orbColor;
uniform float intensity;
uniform vec3 centerPosition; // Position of the orb's center
uniform float radius;        // Radius of the orb

void main() {
    // Calculate distance from the center
   vec3 pos = vec3(outModel * vec4(centerPosition, 1.0));
    float distance = length(fragPosition - pos);
    float edgeFactor = smoothstep(radius * 0.9, radius, distance); // Smooth transition

    vec3 color = mix(orbColor * intensity, vec3(0.0), edgeFactor);
    FragColor = vec4(color, 1.0);
}
