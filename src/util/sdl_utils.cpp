#include "./sdl_utils.h"
#include "./sdl_font_cache.h"

int detect_line_height(TTF_Font *font)
{
    int w, h;
    if (TTF_SizeUTF8(font, "A", &w, &h) == 0)
    {
        return h;
    }
    return 24;
}

int detect_line_height(const std::string &font, uint32_t size)
{
    return detect_line_height(cached_load_font(font, size));
}
