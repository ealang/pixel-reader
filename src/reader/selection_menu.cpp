#include "./selection_menu.h"

#include "sys/screen.h"
#include "sys/keymap.h"
#include "./sdl_utils.h"

SelectionMenu::SelectionMenu(TTF_Font *font)
    : SelectionMenu({}, font)
{
}

SelectionMenu::SelectionMenu(std::vector<std::string> entries, TTF_Font *font)
    : entries(entries),
      font(font),
      line_height(detect_line_height(font)),
      num_display_lines((SCREEN_HEIGHT - line_padding) / (line_height + line_padding))
{
}

SelectionMenu::~SelectionMenu()
{
}

void SelectionMenu::set_entries(std::vector<std::string> new_entries)
{
    entries = new_entries;
    cursor_pos = 0;
    scroll_pos = 0;
}

void SelectionMenu::set_on_selection(std::function<void(uint32_t)> _on_selection)
{
    on_selection = _on_selection;
}

void SelectionMenu::set_cursor_pos(uint32_t pos)
{
    if (pos < entries.size())
    {
        cursor_pos = pos;
        // TODO: improve this
        scroll_pos = std::max(0, static_cast<int>(pos) - (num_display_lines / 2));
    }
}

bool SelectionMenu::render(SDL_Surface *dest_surface)
{
    const SDL_PixelFormat *pixel_format = dest_surface->format;

    SDL_Color fg_color = {255, 255, 255, 0};
    SDL_Color bg_color = {0, 0, 0, 0};
    SDL_Color highlight_color = {128, 0, 128, 0};

    uint32_t rect_bg_color = SDL_MapRGB(pixel_format, 0, 0, 0);
    uint32_t rect_highlight_color = SDL_MapRGB(pixel_format, 128, 0, 128);

    Sint16 x = line_padding;
    Sint16 y = line_padding;

    // Clear screen
    SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_FillRect(dest_surface, &rect, rect_bg_color);

    // Draw lines
    for (int i = 0; i < num_display_lines; ++i)
    {
        int global_i = i + scroll_pos;
        if (global_i >= (int)entries.size())
        {
            break;
        }

        const auto &entry = entries[global_i];

        bool is_highlighted = (global_i == cursor_pos);

        // Draw hightlight
        if (is_highlighted)
        {
            SDL_Rect rect = {0, (Sint16)(y - line_padding / 2), SCREEN_WIDTH, (Uint16)(line_height + line_padding)};
            SDL_FillRect(dest_surface, &rect, rect_highlight_color);
        }

        // Draw text
        {
            SDL_Rect rectMessage = {x, y, 0, 0};
            SDL_Surface *message = TTF_RenderUTF8_Shaded(font, entry.c_str(), fg_color, is_highlighted ? highlight_color : bg_color);
            SDL_BlitSurface(message, NULL, dest_surface, &rectMessage);
            SDL_FreeSurface(message);
        }

        y += line_height + line_padding;
    }

    return true;
}

void SelectionMenu::on_move_down()
{
    int num_entries = entries.size();
    if (cursor_pos < num_entries - 1)
    {
        cursor_pos++;
        if (cursor_pos >= scroll_pos + num_display_lines)
        {
            scroll_pos = cursor_pos - num_display_lines + 1;
        }
    }
}

void SelectionMenu::on_move_up()
{
    cursor_pos = std::max(cursor_pos - 1, 0);
    scroll_pos = std::min(scroll_pos, cursor_pos);
}

void SelectionMenu::on_select_entry()
{
    if (!entries.empty() && on_selection)
    {
        on_selection(cursor_pos);
    }
}

bool SelectionMenu::on_keypress(SDLKey key)
{
    switch (key) {
        case SW_BTN_DOWN:
            on_move_down();
            break;
        case SW_BTN_UP:
            on_move_up();
            break;
        case SW_BTN_A:
            on_select_entry();
            break;
        case SW_BTN_B:
            _is_done = true;
            break;
        default:
            break;
    }
    return true;
}

bool SelectionMenu::is_done()
{
    return _is_done;
}
