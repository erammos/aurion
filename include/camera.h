#pragma once

#include "cglm/types.h"
#include "cglm/vec3.h"

typedef struct {
    vec3 pos;
    vec3 up;
    vec3 front;
    vec3 right;
    float yaw;
    float pitch;
    mat4 view;
    mat4 projection;
} g_camera;

g_camera camera_create(int, int);
void camera_update(g_camera* camera, vec3 pos, vec3 up, float yaw, float pitch);
void camera_set_perspective(g_camera* camera, float fov, float aspect, float near, float far);
void camera_animate(g_camera* camera, vec3 mouse_pos, vec3 input_axis, float speed, float sensitivity, float dt);