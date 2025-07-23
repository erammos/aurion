#pragma once

#include "cglm/cglm.h"

typedef struct {
    vec3 pos;
    vec3 up;
    vec3 front;
    vec3 right;
    float yaw;
    float pitch;
    float roll;
    mat4 view;
    mat4 projection;
} g_camera;

g_camera camera_create(int, int);
void camera_update(g_camera* camera, vec3 pos, vec3 up, float yaw, float pitch);
void camera_set_perspective(g_camera* camera, float fov, float aspect, float near, float far);
void camera_free_animate(g_camera* camera, vec3 mouse_pos, vec3 input_axis, float speed, float sensitivity, float dt);
void camera_locked_animate(g_camera* camera, vec3 mouse_pos, vec3 input_axis, float speed, float sensitivity, float dt);
void camera_look_at(g_camera* camera, vec3 target, vec3 up);
