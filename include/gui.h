#pragma once
#include <SDL2/SDL.h>

void gui_init(SDL_Window* win);
void gui_draw_text(int x, int y, char* text);
void gui_input_begin();
void gui_input_end();
void gui_input(SDL_Event event);
void gui_draw_fps();
void gui_begin();
void gui_end();