#include "./sdl_pointer.h"

void SDL_Deleter::operator()(TTF_Font* font) {
    if (font)
    {
        TTF_CloseFont(font);
    }
}

void SDL_Deleter::operator()(SDL_Surface* surface) {
    if (surface)
    {
        SDL_FreeSurface(surface);
    }
}
