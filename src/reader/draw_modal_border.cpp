#include "./draw_modal_border.h"

#include "sys/screen.h"
#include "util/sdl_pointer.h"

void draw_modal_border(uint32_t w, uint32_t h, const ColorTheme &theme, SDL_Surface *dest_surface)
{
    Uint16 dialog_padding = 25;
    Uint16 line_width = 3;

    w += dialog_padding * 2;
    h += dialog_padding * 2;

    // transparent background
    {
        surface_unique_ptr mask = surface_unique_ptr {
            SDL_CreateRGBSurface(
                SDL_SWSURFACE,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                32,
                0, 0, 0, 0
            )
        };
        SDL_SetAlpha(mask.get(), SDL_SRCALPHA, 128);

        SDL_Rect rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_FillRect(
            mask.get(),
            &rect,
            SDL_MapRGB(mask->format, 0, 0, 0)
        );
        SDL_BlitSurface(mask.get(), NULL, dest_surface, &rect);
    }

    // draw border
    {
        const auto &border_color = theme.main_text;
        const auto &bg_color = theme.background;

        SDL_Rect rect = {
            static_cast<Sint16>(SCREEN_WIDTH / 2 - w / 2),
            static_cast<Sint16>(SCREEN_HEIGHT / 2 - h / 2),
            static_cast<Uint16>(w),
            static_cast<Uint16>(h)
        };
        SDL_FillRect(
            dest_surface,
            &rect,
            SDL_MapRGB(dest_surface->format, border_color.r, border_color.g, border_color.b)
        );

        rect.x += line_width;
        rect.y += line_width;
        rect.w -= line_width * 2;
        rect.h -= line_width * 2;

        SDL_FillRect(
            dest_surface,
            &rect,
            SDL_MapRGB(dest_surface->format, bg_color.r, bg_color.g, bg_color.b)
        );
    }
}
