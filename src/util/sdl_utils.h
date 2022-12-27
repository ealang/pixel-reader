#ifndef SDL_UTILS_H_
#define SDL_UTILS_H_

#include "./sdl_pointer.h"
#include <SDL/SDL_ttf.h>
#include <string>

int detect_line_height(TTF_Font *font);
int detect_line_height(const std::string &font, uint32_t size);

surface_unique_ptr load_surface_from_ptr(const char *data, uint32_t size, const std::string &img_format, SDL_PixelFormat *surface_format);

#endif
