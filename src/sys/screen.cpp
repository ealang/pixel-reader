#include "./screen.h"

#include <SDL/SDL_video.h>
#include <stdexcept>

static bool format_initialized = false;
static SDL_PixelFormat format;
short unsigned int SCREEN_WIDTH  = 640;
short unsigned int SCREEN_HEIGHT = 480;

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
