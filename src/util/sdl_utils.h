#ifndef SDL_UTILS_H_
#define SDL_UTILS_H_

#include <SDL/SDL_ttf.h>
#include <string>

int detect_line_height(TTF_Font *font);
int detect_line_height(const std::string &font, uint32_t size);

#endif
