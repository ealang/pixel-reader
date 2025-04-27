#ifndef SCREEN_H_
#define SCREEN_H_

#include <SDL2/SDL.h>

extern short unsigned int SCREEN_WIDTH;
extern short unsigned int SCREEN_HEIGHT;

// Global SDL2 renderer and window
extern SDL_Renderer* g_renderer;
extern SDL_Window* g_window;
extern SDL_Surface* g_screen;

// Initialize SDL2 window and renderer
bool initialize_sdl2(const char* title);

// Clean up SDL2 resources
void cleanup_sdl2();

// For compatibility with SDL 1.2 code that depends on pixel format
SDL_PixelFormat* get_render_surface_format();
void set_render_surface_format(SDL_PixelFormat* format);

#endif
