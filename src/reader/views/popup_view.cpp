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

    auto text = surface_unique_ptr { TTF_RenderUTF8_Shaded(
        font,
        message.c_str(),
        theme.main_text,
        theme.background
    ) };

    draw_modal_border(
        text->w,
        text->h,
        styling.get_loaded_color_theme(),
        dest_surface
    );

    SDL_Rect rect = {
        static_cast<Sint16>(SCREEN_WIDTH / 2 - text->w / 2),
        static_cast<Sint16>(SCREEN_HEIGHT / 2 - text->h / 2),
        0,
        0
    };
    SDL_BlitSurface(text.get(), NULL, dest_surface, &rect);

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

void PopupView::on_keypress(SDLKey)
{
    _is_done = true;
}
