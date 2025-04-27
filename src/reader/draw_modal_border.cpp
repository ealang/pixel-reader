#include "./draw_modal_border.h"

#include "sys/screen.h"
#include "util/sdl_pointer.h"
#include "./config.h"

void draw_modal_border(uint32_t w, uint32_t h, const ColorTheme &theme, SDL_Surface *surface)
{
    w += DIALOG_PADDING * 2;
    h += DIALOG_PADDING * 2;

    const auto &border_color = theme.main_text;
    const auto &bg_color = theme.background;
    
    // Create pixel values
    Uint32 border_pixel = SDL_MapRGB(surface->format, border_color.r, border_color.g, border_color.b);
    Uint32 bg_pixel = SDL_MapRGB(surface->format, bg_color.r, bg_color.g, bg_color.b);
    Uint32 overlay_pixel = SDL_MapRGBA(surface->format, 0, 0, 0, 128);

    // Overlay for whole screen (simplified - no alpha support for surfaces)
    SDL_Rect rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_FillRect(surface, &rect, overlay_pixel);

    // Draw outer border rectangle (main text color)
    rect = {
        static_cast<int>(SCREEN_WIDTH / 2 - w / 2),
        static_cast<int>(SCREEN_HEIGHT / 2 - h / 2),
        static_cast<int>(w),
        static_cast<int>(h)
    };
    
    SDL_FillRect(surface, &rect, border_pixel);

    // Draw inner rectangle (background color)
    rect.x += DIALOG_BORDER_WIDTH;
    rect.y += DIALOG_BORDER_WIDTH;
    rect.w -= DIALOG_BORDER_WIDTH * 2;
    rect.h -= DIALOG_BORDER_WIDTH * 2;

    SDL_FillRect(surface, &rect, bg_pixel);
}