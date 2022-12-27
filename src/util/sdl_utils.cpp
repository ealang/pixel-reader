#include "./sdl_utils.h"
#include "./sdl_font_cache.h"

#include <SDL/SDL_image.h>
#include <iostream>

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

surface_unique_ptr load_surface_from_ptr(const char *data, uint32_t size, const std::string &img_format, SDL_PixelFormat *surface_format)
{
    char type_str[8];
    {
        strncpy(type_str, img_format.c_str(), sizeof(type_str));
        type_str[sizeof(type_str) - 1] = 0;
        for (auto &c: type_str)
        {
            c = std::toupper(c);
        }
    }

    SDL_RWops *rw = SDL_RWFromConstMem(data, size);
    auto loaded_surface = surface_unique_ptr { IMG_LoadTyped_RW(rw, 0, type_str) };
    if (loaded_surface == nullptr)
    {
        std::cerr << "Failed to load image of type " << type_str << ": " << IMG_GetError() << std::endl;
        return nullptr;
    }

    return surface_unique_ptr { SDL_ConvertSurface(loaded_surface.get(), surface_format, 0) };
}
