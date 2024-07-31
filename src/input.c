#include "input.h"
#include "gui.h"
bool keyStates[SDL_NUM_SCANCODES] = {false};

void
input_keydown(SDL_Event* event, bool* running) {
    if (event->key.keysym.sym == SDLK_ESCAPE) {
        *running = false;
    } else {
        keyStates[event->key.keysym.scancode] = true;
    }
}

void
input_keyup(SDL_Event* event) {
    keyStates[event->key.keysym.scancode] = false;
}

void
input_mousemotion(SDL_Event* event, float* mouse_pos) {
    mouse_pos[0] = event->motion.xrel;
    mouse_pos[1] = event->motion.yrel;
}

void
input_update_movement_axis(float* input_axis) {
    input_axis[0] = (keyStates[SDL_SCANCODE_D] ? 1.0f : 0.0f) - (keyStates[SDL_SCANCODE_A] ? 1.0f : 0.0f);
    input_axis[1] = (keyStates[SDL_SCANCODE_W] ? 1.0f : 0.0f) - (keyStates[SDL_SCANCODE_S] ? 1.0f : 0.0f);
}

bool
input_update(float* input_axis, float* mouse_pos) {
    bool running = true;
    SDL_Event event;
    gui_input_begin();
    mouse_pos[0] = 0;
    mouse_pos[1] = 0;
    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
            input_keydown(&event, &running);
        } else if (event.type == SDL_KEYUP) {
            input_keyup(&event);
        } else if (event.type == SDL_MOUSEMOTION) {

            input_mousemotion(&event, mouse_pos);
        }
        gui_input(event);
    }
    input_update_movement_axis(input_axis);

    gui_input_end();
    return running;
}