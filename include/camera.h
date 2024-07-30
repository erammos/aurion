#pragma once

#include "cglm/vec3.h"
typedef struct {
    vec3 position;
    vec3 front;
    vec3 right;
    vec3 up;
    vec3 target;
    mat4 view;
} g_camera;

g_camera
camera_create(vec3 pos, vec3 up, float yaw, float pitch);