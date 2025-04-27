#include "./screen.h"

#include <SDL2/SDL.h>
#include <stdexcept>
#include <iostream>

static bool format_initialized = false;
static SDL_PixelFormat format;
short unsigned int SCREEN_WIDTH  = 640;
short unsigned int SCREEN_HEIGHT = 480;

// Global SDL2 objects
SDL_Renderer* g_renderer = nullptr;
SDL_Window* g_window = nullptr;
SDL_Surface* g_screen = nullptr;

// For compatibility with code that depends on SDL 1.2 pixel format
SDL_Surface* g_compatible_surface = nullptr;

bool initialize_sdl2(const char* title)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create window
    g_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!g_window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create renderer
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    if (!g_renderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize renderer color
    SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    // Create a compatible surface for legacy code
    g_compatible_surface = SDL_CreateRGBSurface(
        0,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        32, 0, 0, 0, 0
    );

    if (!g_compatible_surface) {
        std::cerr << "Compatible surface could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Set g_screen to the compatible surface for View rendering
    g_screen = g_compatible_surface;

    // Set the pixel format
    set_render_surface_format(g_compatible_surface->format);

    return true;
}

void cleanup_sdl2()
{
    // Reset g_screen to nullptr first (it points to g_compatible_surface)
    g_screen = nullptr;
    
    if (g_compatible_surface) {
        SDL_FreeSurface(g_compatible_surface);
        g_compatible_surface = nullptr;
    }

    if (g_renderer) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = nullptr;
    }

    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = nullptr;
    }

    SDL_Quit();
}

SDL_PixelFormat *get_render_surface_format()
{
    if (!format_initialized)
    {
        throw std::runtime_error("Render surface format not initialized");
    }
    return &format;
}

void set_render_surface_format(SDL_PixelFormat *new_format)
{
    format_initialized = true;
    format = *new_format;
}
