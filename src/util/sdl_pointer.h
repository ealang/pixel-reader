#ifndef SDL_POINTER_H_
#define SDL_POINTER_H_

#include <SDL2/SDL_ttf.h>
#include <memory>

struct SDL_Deleter {
  void operator()(TTF_Font* font);
  void operator()(SDL_Surface* surface);
  void operator()(SDL_Texture* texture);
  void operator()(SDL_Renderer* renderer);
  void operator()(SDL_Window* window);
};

using ttf_font_unique_ptr = std::unique_ptr<TTF_Font, SDL_Deleter>;
using surface_unique_ptr = std::unique_ptr<SDL_Surface, SDL_Deleter>;
using texture_unique_ptr = std::unique_ptr<SDL_Texture, SDL_Deleter>;
using renderer_unique_ptr = std::unique_ptr<SDL_Renderer, SDL_Deleter>;
using window_unique_ptr = std::unique_ptr<SDL_Window, SDL_Deleter>;

#endif
