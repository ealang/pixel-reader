#ifndef SDL_POINTER_H_
#define SDL_POINTER_H_

#include <SDL/SDL_ttf.h>
#include <memory>

struct SDL_Deleter {
  void operator()(TTF_Font* font);
};

using ttf_font_unique_ptr = std::unique_ptr<TTF_Font, SDL_Deleter>;

#endif
