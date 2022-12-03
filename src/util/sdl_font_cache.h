#ifndef SDL_FONT_CACHE_H_
#define SDL_FONT_CACHE_H_

#include <SDL/SDL_ttf.h>
#include <string>

enum class FontLoadErrorOpt
{
    NoThrow,
    ThrowOnError,
};

TTF_Font *cached_load_font(const std::string &font_path, uint32_t size, FontLoadErrorOpt opt = FontLoadErrorOpt::ThrowOnError);

#endif
