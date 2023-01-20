#include "./settings_view.h"

#include "reader/color_theme_def.h"
#include "reader/draw_modal_border.h"
#include "reader/font_catalog.h"
#include "reader/settings_store.h"
#include "reader/shoulder_keymap.h"
#include "reader/system_styling.h"
#include "reader/views/token_view/token_view_styling.h"
#include "sys/keymap.h"
#include "sys/screen.h"
#include "util/sdl_font_cache.h"
#include "util/sdl_utils.h"

#include <filesystem>

SettingsView::SettingsView(
    SystemStyling &sys_styling,
    TokenViewStyling &token_view_styling,
    std::string font_name
) : font_name(font_name),
    sys_styling(sys_styling),
    styling_sub_id(sys_styling.subscribe_to_changes([this]() {
        needs_render = true;
    })),
    token_view_styling(token_view_styling)
{
}

SettingsView::~SettingsView()
{
    sys_styling.unsubscribe_from_changes(styling_sub_id);
}

bool SettingsView::render(SDL_Surface *dest_surface, bool force_render)
{
    if (needs_render || force_render)
    {
        TTF_Font *font = cached_load_font(font_name, sys_styling.get_font_size());
        const auto &theme = sys_styling.get_loaded_color_theme();
        const auto &text_color = theme.main_text;
        const auto &bg_color = theme.background;
        const auto &hl_text_color = theme.highlight_text;
        const auto &hl_bg_color = theme.highlight_background;

        auto arrowify = [](const std::string &str, bool is_selected) {
            if (!is_selected)
            {
                return str;
            }
            return "◂" + str + "▸";
        };

        auto render_text = [&](const char *str, bool is_selected = false) {
            return surface_unique_ptr { TTF_RenderUTF8_Shaded(
                font,
                str,
                is_selected ? hl_text_color : text_color,
                is_selected ? hl_bg_color : bg_color
            ) };
        };

        auto theme_label_surf = surface_unique_ptr {
            TTF_RenderUTF8_Shaded(font, "Theme:", theme.secondary_text, bg_color)
        };
        auto theme_value_surf = [&](bool selected = true) {
            return render_text(
                arrowify(
                    sys_styling.get_color_theme(),
                    selected
                ).c_str(),
                selected
            );
        };

        auto font_size_label_surf = surface_unique_ptr {
            TTF_RenderUTF8_Shaded(font, "Font size:", theme.secondary_text, bg_color)
        };
        auto font_size_value_surf = [&](bool selected = true) {
            return render_text(
                arrowify(
                    std::to_string(sys_styling.get_font_size()),
                    selected
                ).c_str(),
                selected
            );
        };

        auto font_name_label_surf = surface_unique_ptr {
            TTF_RenderUTF8_Shaded(font, "Font:", theme.secondary_text, bg_color)
        };
        auto font_name_value_surf = [&](bool selected = true) {
            return render_text(
                arrowify(
                    std::filesystem::path(sys_styling.get_font_name()).filename().stem().string(),
                    selected
                ).c_str(),
                selected
            );
        };

        auto shoulder_keymap_label_surf = surface_unique_ptr {
            TTF_RenderUTF8_Shaded(font, "Shoulder keymap:", theme.secondary_text, bg_color)
        };
        auto shoulder_keymap_value_surf = [&](bool selected = true) {
            const auto &text = get_shoulder_keymap_display_name(
                token_view_styling.get_shoulder_keymap()
            );
            return render_text(
                arrowify(
                    text,
                    selected
                ).c_str(),
                selected
            );
        };

        Uint16 text_padding = 5;

        Uint16 content_w = std::max(
            std::max(
                std::max(
                    font_size_value_surf().get()->w,
                    theme_value_surf()->w
                ),
                std::max(
                    font_name_value_surf().get()->w,
                    font_size_label_surf->w
                )
            ),
            std::max(
                shoulder_keymap_label_surf->w,
                shoulder_keymap_value_surf().get()->w
            )
        );
        Uint16 content_h = (
            theme_label_surf->h +
            theme_value_surf()->h +
            text_padding +
            font_size_label_surf->h +
            font_size_value_surf()->h +
            text_padding +
            font_name_label_surf->h +
            font_name_value_surf().get()->h +
            text_padding +
            shoulder_keymap_label_surf->h +
            shoulder_keymap_value_surf().get()->h
        );
        Sint16 content_y = SCREEN_HEIGHT / 2 - content_h / 2;

        draw_modal_border(
            content_w,
            content_h,
            sys_styling.get_loaded_color_theme(),
            dest_surface
        );

        // draw text
        {
            SDL_Rect rect = {0, content_y, 0, 0};
            auto push_text = [&](SDL_Surface *surf) {
                rect.x = SCREEN_WIDTH / 2 - surf->w / 2;
                SDL_BlitSurface(surf, NULL, dest_surface, &rect);
                rect.y += surf->h;
            };

            push_text(theme_label_surf.get());
            push_text(theme_value_surf(line_selected == 0).get());
            rect.y += text_padding;

            push_text(font_size_label_surf.get());
            push_text(font_size_value_surf(line_selected == 1).get());
            rect.y += text_padding;

            push_text(font_name_label_surf.get());
            push_text(font_name_value_surf(line_selected == 2).get());
            rect.y += text_padding;

            push_text(shoulder_keymap_label_surf.get());
            push_text(shoulder_keymap_value_surf(line_selected == 3).get());
        }

        needs_render = false;
        return true;
    }

    return false;
}

bool SettingsView::is_done()
{
    return _is_done;
}

bool SettingsView::is_modal()
{
    return true;
}

void SettingsView::on_change_theme(int dir)
{
    std::string theme = sys_styling.get_color_theme();
    sys_styling.set_color_theme(
        (dir < 0) ?
            get_prev_theme(theme) :
            get_next_theme(theme)
    );
}

void SettingsView::on_change_font_size(int dir)
{
    sys_styling.set_font_size(
        (dir < 0) ?
            sys_styling.get_prev_font_size() :
            sys_styling.get_next_font_size()
    );
}

void SettingsView::on_change_font_name(int dir)
{
    std::string font_name = sys_styling.get_font_name();
    sys_styling.set_font_name(
        (dir < 0) ?
            get_prev_font_name(font_name) :
            get_next_font_name(font_name)
    );
}

void SettingsView::on_change_shoulder_keymap(int dir)
{
    const auto &keymap = token_view_styling.get_shoulder_keymap();
    token_view_styling.set_shoulder_keymap(
        (dir < 0) ?
            get_prev_shoulder_keymap(keymap) :
            get_next_shoulder_keymap(keymap)
    );
}

void SettingsView::on_keypress(SDLKey key)
{
    static const uint32_t n = 4;
    switch (key) {
        case SW_BTN_UP:
            line_selected = (line_selected + n - 1) % n;
            needs_render = true;
            break;
        case SW_BTN_DOWN:
            line_selected = (line_selected + 1) % n;
            needs_render = true;
            break;
        case SW_BTN_LEFT:
        case SW_BTN_RIGHT:
            {
                int dir = (key == SW_BTN_LEFT) ? -1 : 1;
                if (line_selected == 0)
                {
                    on_change_theme(dir);
                }
                else if (line_selected == 1)
                {
                    on_change_font_size(dir);
                }
                else if (line_selected == 2)
                {
                    on_change_font_name(dir);
                }
                else
                {
                    on_change_shoulder_keymap(dir);
                }
                needs_render = true;
            }
            break;
        case SW_BTN_B:
            {
                _is_done = true;
            }
            break;
        default:
            break;
    }
}

void SettingsView::terminate()
{
    _is_done = true;
}

void SettingsView::unterminate()
{
    _is_done = false;
}
