#include "./sdl_font_cache.h"
#include "./sdl_pointer.h"

#include <iostream>
#include <unordered_map>

using fonts_lookup = std::unordered_map<std::string, ttf_font_unique_ptr>;
static std::unordered_map<uint32_t, fonts_lookup> font_cache;

static TTF_Font *load_with_warning(const std::string &font, uint32_t size, FontLoadErrorOpt opt)
{
    TTF_Font *font_ptr = TTF_OpenFont(font.c_str(), size);
    if (!font_ptr)
    {
        std::cerr << "Failed to load font: " << font << " " << size << std::endl;

        if (opt == FontLoadErrorOpt::ThrowOnError)
        {
            throw std::runtime_error("Failed to load font");
        }
    }
    return font_ptr;
}

TTF_Font *cached_load_font(const std::string &font_path, uint32_t size, FontLoadErrorOpt opt)
{
    auto &lookup = font_cache[size];

    auto it = lookup.find(font_path);
    if (it == lookup.end())
    {
        lookup.emplace(font_path, load_with_warning(font_path.c_str(), size, opt));
        return lookup.at(font_path).get();
    }

    return it->second.get();
}
