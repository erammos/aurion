#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    // This trick removes the translation part of the view matrix,
    // so the skybox always stays centered on the camera.
    mat4 view_no_translation = mat4(mat3(view));
    vec4 pos = projection * view_no_translation * vec4(aPos, 1.0);
    // Set z to w to make the depth value always 1.0 (the furthest possible)
    gl_Position = pos.xyww;
}