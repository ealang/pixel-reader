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

void SDL_Deleter::operator()(SDL_Texture* texture) {
    if (texture)
    {
        SDL_DestroyTexture(texture);
    }
}

void SDL_Deleter::operator()(SDL_Renderer* renderer) {
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
    }
}

void SDL_Deleter::operator()(SDL_Window* window) {
    if (window)
    {
        SDL_DestroyWindow(window);
    }
}
