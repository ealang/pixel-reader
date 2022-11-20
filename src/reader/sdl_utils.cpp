#include "./sdl_utils.h"

int detect_line_height(TTF_Font *font)
{
    int w, h;
    if (TTF_SizeUTF8(font, "A", &w, &h) == 0)
    {
        return h;
    }
    return 24;
}
