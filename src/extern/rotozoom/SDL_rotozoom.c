// Stub implementation for SDL_rotozoom
// This replaces the original implementation with simple stubs to allow compilation

#include "./SDL_rotozoom.h"

SDL_Surface *rotozoomSurface(SDL_Surface *src, double angle, double zoom, int smooth)
{
    // Return a simple copy of the surface as a fallback
    return SDL_ConvertSurface(src, src->format, 0);
}

SDL_Surface *zoomSurface(SDL_Surface *src, double zoomx, double zoomy, int smooth)
{
    // Return a simple copy of the surface as a fallback
    return SDL_ConvertSurface(src, src->format, 0);
}