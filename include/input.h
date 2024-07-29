#pragma once
#include <SDL2/SDL.h>
void input_keydown(SDL_Event* event, bool* running);
void input_keyup(SDL_Event* event);
void input_mousemotion(SDL_Event* event, int* mouse_pos);
void input_update_movement_axis(float* input_axis);
bool input_update(float* input_axis, int* mouse_pos);