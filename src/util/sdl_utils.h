#ifndef SDL_UTILS_H_
#define SDL_UTILS_H_

#include "./sdl_pointer.h"
#include <SDL2/SDL_ttf.h>
#include <string>

int detect_line_height(TTF_Font *font);
int detect_line_height(const std::string &font, uint32_t size);

surface_unique_ptr load_surface_from_ptr(const char *data, uint32_t size, const std::string &img_format, SDL_PixelFormat *surface_format);

// SDL2 utility functions
texture_unique_ptr surface_to_texture(SDL_Surface* surface, SDL_Renderer* renderer);
texture_unique_ptr load_texture_from_ptr(const char *data, uint32_t size, const std::string &img_format, SDL_Renderer* renderer);
texture_unique_ptr render_text_to_texture(TTF_Font* font, const char* text, SDL_Color color, SDL_Renderer* renderer);

// Helper for blitting in SDL2
void blit_texture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect = nullptr, const SDL_Rect* dstrect = nullptr);

#endif
