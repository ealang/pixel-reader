#include "./sdl_pointer.h"

void SDL_Deleter::operator()(TTF_Font* font) {
    if (font)
    {
        TTF_CloseFont(font);
    }
}
