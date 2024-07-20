#include <assert.h>
#include <glad/glad.h>
#include <gui.h>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#define NK_IMPLEMENTATION
#define NK_INCLUDE_DEFAULT_ALLOCATOR

#include <nuklear.h>
#include "nuklear_sdl_gl3.h"
#define MAX_VERTEX_MEMORY  512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024
static struct nk_context* ctx;
int screen_width, screen_height;

void
gui_init(SDL_Window* win) {
    SDL_GetWindowSize(win, &screen_width, &screen_height);
    ctx = nk_sdl_init(win);
    {
        struct nk_font_atlas* atlas;
        nk_sdl_font_stash_begin(&atlas);
        /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
        /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16, 0);*/
        /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
        /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
        /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
        /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
        nk_sdl_font_stash_end();
    /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
    /*nk_style_set_font(ctx, &roboto->handle);*/}
    struct nk_color clear = nk_rgba(0, 0, 0, 0);
    ctx->style.window.fixed_background = nk_style_item_color(clear);
}

void
gui_input_begin() {
    nk_input_begin(ctx);
}

void
gui_input_end() {
    nk_input_end(ctx);
}

void
gui_input(SDL_Event event) {
    nk_sdl_handle_event(&event);
}

void
gui_begin() {
    nk_begin(ctx, "", nk_rect(0, 0, screen_width, screen_height), NK_WINDOW_NO_INPUT);
}

void
gui_end() {
    nk_end(ctx);
    nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
}

void
gui_draw_text(int x, int y, char* text) {

    // Directly draw text using Nuklear's command buffer
    struct nk_command_buffer* canvas = nk_window_get_canvas(ctx);
    struct nk_rect text_rect = nk_rect(x, y, 200, 40);
    struct nk_color text_color = nk_rgba(255, 255, 255, 255); // Opaque white text
    nk_draw_text(canvas, text_rect, text, strlen(text), ctx->style.font, nk_rgba(0, 0, 0, 0), text_color);
}
