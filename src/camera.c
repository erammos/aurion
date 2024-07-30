#include "camera.h"
#include "cglm/cam.h"
g_camera
camera_create(vec3 pos, vec3 up, float yaw, float pitch) {

    g_camera camera = {};
    mat4 view;
    camera.front[0] = cos(radians(yaw)) * cos(radians(pitch));
    camera.front[1] = sin(radians(pitch));
    camera.front[2] = sin(radians(yaw)) * cos(radians(pitch));

    glm_vec3_copy(pos, camera.position);
    glm_normalize(camera.front);
    glm_cross(camera.front, up, camera.right);
    glm_normalize(camera.right);
    glm_cross(camera.right, camera.front, camera.up);
    glm_normalize(camera.up);

    glm_vec3_add(camera.position, camera.front, camera.target);
    glm_lookat(camera.position, camera.target, camera.up, view);
}

