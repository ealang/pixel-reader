#include "./popup_view.h"

#include "reader/draw_modal_border.h"
#include "reader/system_styling.h"
#include "sys/screen.h"
#include "util/sdl_font_cache.h"
#include "util/sdl_utils.h"

PopupView::PopupView(const std::string &message, std::string font_name, SystemStyling &styling)
    : message(message)
    , font_name(font_name)
    , styling(styling)
    , styling_sub_id(styling.subscribe_to_changes([this](SystemStyling::ChangeId) {
          _needs_render = true;
      }))
{
}

PopupView::~PopupView()
{
    styling.unsubscribe_from_changes(styling_sub_id);
}

bool PopupView::render(SDL_Surface *dest_surface, bool force_render)
{
    if (!_needs_render && !force_render)
    {
        return false;
    }

    TTF_Font *font = cached_load_font(font_name, styling.get_font_size());
    const auto &theme = styling.get_loaded_color_theme();

    // Simplified implementation for SDL2 conversion
    SDL_Surface *text_surface = TTF_RenderText_Blended(
        font, 
        message.c_str(), 
        theme.main_text
    );
    
    if (!text_surface) {
        return false;
    }
    
    // Get text dimensions
    int text_width = text_surface->w;
    int text_height = text_surface->h;

    // Draw text to destination
    SDL_Rect rect = {
        static_cast<int>(SCREEN_WIDTH / 2 - text_width / 2),
        static_cast<int>(SCREEN_HEIGHT / 2 - text_height / 2),
        text_width,
        text_height
    };
    
    SDL_BlitSurface(text_surface, NULL, dest_surface, &rect);
    SDL_FreeSurface(text_surface);

    _needs_render = false;
    return true;
}

bool PopupView::is_done()
{
    return _is_done;
}

bool PopupView::is_modal()
{
    return true;
}

void PopupView::on_keypress(SDL_Keycode)
{
    _is_done = true;
}