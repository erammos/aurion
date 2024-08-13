#include "camera.h"
#include "cglm/affine-pre.h"
#include "cglm/io.h"
#include "cglm/mat4.h"
#include "defines.h"

g_camera
camera_create(int screen_width, int screen_height) {

    g_camera camera = {};
    camera_update(&camera, (vec3){0, 0, -5}, (vec3){0, 1, 0}, 0, 0);
    camera_set_perspective(&camera, DEG2RAD(45), screen_width / (float)screen_height, 0.1, 1000);
    return camera;
}

void
camera_update(g_camera* camera, vec3 pos, vec3 up, float yaw, float pitch) {
    camera->yaw = yaw;
    camera->pitch = pitch;
    camera->pitch = glm_clamp(camera->pitch, -89, 89);
    camera->roll = 0;
    glm_mat4_identity(camera->view);
    glm_euler((vec3){glm_rad(camera->pitch), glm_rad(camera->yaw), glm_rad((camera->roll))}, camera->view);
    glm_translate(camera->view ,(vec3){-pos[0], -pos[1], -pos[2]});
    glm_vec3_copy(pos, camera->pos);
}
void
camera_set_perspective(g_camera* camera, float fov, float aspect, float near, float far) {
    glm_perspective(fov, aspect, near, far, camera->projection);
};

void
camera_free_animate(g_camera* camera, vec3 mouse_pos, vec3 input_axis, float speed, float sensitivity, float dt) {

    vec3 right = {camera->view[0][0], camera->view[1][0], camera->view[2][0]};
    vec3 front = {camera->view[0][2], camera->view[1][2], camera->view[2][2]};

    glm_vec3_scale(front, -speed * dt * input_axis[1], front);
    glm_vec3_scale(right, speed * dt * input_axis[0], right);
    glm_vec3_add(camera->pos, front, camera->pos);
    glm_vec3_add(camera->pos, right, camera->pos);
    camera_update(camera, camera->pos, (vec3){0, 1, 0}, camera->yaw + sensitivity * mouse_pos[0],
                  camera->pitch + sensitivity * mouse_pos[1]);
}

void
camera_locked_animate(g_camera* camera, vec3 mouse_pos, vec3 input_axis, float speed, float sensitivity, float dt) {

    vec3 right = {camera->view[0][0], camera->view[1][0], camera->view[2][0]};
    vec3 front = {camera->view[0][2], camera->view[1][2], camera->view[2][2]};

    glm_vec3_scale(front, -speed * dt * input_axis[1], front);
    glm_vec3_scale(right, speed * dt * input_axis[0], right);
    camera->pos[0] += front[0];
    camera->pos[2] += front[2];
    camera->pos[0] += right[0];
    camera->pos[2] += right[2];
    camera_update(camera, camera->pos, (vec3){0, 1, 0}, camera->yaw + sensitivity * mouse_pos[0],
                  camera->pitch + sensitivity * mouse_pos[1]);
}
