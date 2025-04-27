#include "./sdl_utils.h"
#include "./sdl_font_cache.h"
#include "../sys/screen.h"

#include <SDL2/SDL_image.h>
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

// SDL2 utility functions
texture_unique_ptr surface_to_texture(SDL_Surface* surface, SDL_Renderer* renderer)
{
    if (!surface || !renderer)
    {
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    return texture_unique_ptr(texture);
}

texture_unique_ptr load_texture_from_ptr(const char *data, uint32_t size, const std::string &img_format, SDL_Renderer* renderer)
{
    // If no renderer provided, use global renderer
    if (!renderer) {
        renderer = g_renderer;
    }
    
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
    
    return surface_to_texture(loaded_surface.get(), renderer);
}

texture_unique_ptr render_text_to_texture(TTF_Font* font, const char* text, SDL_Color color, SDL_Renderer* renderer)
{
    if (!font || !text)
    {
        return nullptr;
    }
    
    // If no renderer provided, use global renderer
    if (!renderer) {
        renderer = g_renderer;
    }
    
    SDL_Surface* text_surface = TTF_RenderText_Blended(font, text, color);
    if (!text_surface)
    {
        return nullptr;
    }
    
    auto texture = surface_to_texture(text_surface, renderer);
    SDL_FreeSurface(text_surface);
    
    return texture;
}

void blit_texture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect)
{
    if (!texture)
    {
        return;
    }
    
    // If no renderer provided, use global renderer
    if (!renderer) {
        renderer = g_renderer;
    }
    
    SDL_RenderCopy(renderer, texture, srcrect, dstrect);
}
