#include "camera.h"
#include "cglm/cam.h"
#include "defines.h"

g_camera
camera_create(int screen_width, int screen_height) {

    g_camera camera = {};
    camera_update(&camera, (vec3){0, 0, -5}, (vec3){0, 1, 0}, -90, 0);
    camera_set_perspective(&camera, DEG2RAD(45), screen_width / (float)screen_height, 0.1, 1000);
    return camera;
}

static inline void
calculate_front(g_camera* camera) {
    float yaw_rad = DEG2RAD(camera->yaw);
    float pitch_rad = DEG2RAD(camera->pitch);
    camera->front[0] = cos(yaw_rad) * cos(pitch_rad);
    camera->front[1] = sin(pitch_rad);
    camera->front[2] = sin(yaw_rad) * cos(pitch_rad);
    glm_normalize(camera->front);
}

void
camera_update(g_camera* camera, vec3 pos, vec3 up, float yaw, float pitch) {
    camera->yaw = yaw;
    camera->pitch = pitch;
    camera->pitch = glm_clamp(camera->pitch, -89, 89);

    glm_vec3_copy(pos, camera->pos);
    calculate_front(camera);

    glm_cross(camera->front, up, camera->right);
    glm_normalize(camera->right);

    glm_cross(camera->right, camera->front, camera->up);
    glm_normalize(camera->up);

    vec3 target;
    glm_vec3_add(camera->pos, camera->front, target);
    glm_lookat(camera->pos, target, camera->up, camera->view);
}

void
camera_set_perspective(g_camera* camera, float fov, float aspect, float near, float far) {
    glm_perspective(fov, aspect, near, far, camera->projection);
};

void
camera_animate(g_camera* camera, vec3 mouse_pos, vec3 input_axis, float speed, float sensitivity, float dt) {
    vec3 front;
    vec3 right;

    glm_vec3_scale(camera->front, speed * dt * input_axis[1], front);
    glm_vec3_scale(camera->right, speed * dt * input_axis[0], right);
    glm_vec3_add(camera->pos, front, camera->pos);
    glm_vec3_add(camera->pos, right, camera->pos);
    camera_update(camera, camera->pos, (vec3){0, 1, 0}, camera->yaw + sensitivity * mouse_pos[0],
                  camera->pitch - sensitivity * mouse_pos[1]);
}
