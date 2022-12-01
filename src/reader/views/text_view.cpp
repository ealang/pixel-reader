#include "./text_view.h"

#include "reader/text_wrap.h"
#include "reader/sdl_utils.h"
#include "sys/keymap.h"
#include "sys/screen.h"
#include "util/throttled.h"

struct TextViewState
{
    TTF_Font *font;
    const int line_height;
    const int line_padding = 2; // TODO
    const int num_display_lines;

    bool needs_render = true;
    std::vector<std::string> lines;
    int scroll_pos = 0;

    bool show_title_bar = true;
    std::string title;

    std::function<void()> on_resist_up;
    std::function<void()> on_resist_down;

    Throttled line_scroll_throttle;
    Throttled page_scroll_throttle;

    int num_text_display_lines() const
    {
        return num_display_lines - (show_title_bar ? 1 : 0);
    }

    int num_pages() const
    {
        int tot_lines = lines.size();
        int lines_per_screen = num_text_display_lines();
        if (tot_lines == 0 || lines_per_screen == 0)
        {
            return 0;
        }
        if (tot_lines <= lines_per_screen)
        {
            return 1;
        }

        int max_scroll_pos = tot_lines - lines_per_screen;
        return (max_scroll_pos / lines_per_screen) + 1;
    }

    int current_pages() const
    {
        int lines_per_screen = num_text_display_lines();
        if (lines_per_screen == 0)
        {
            return 0;
        }

        return (scroll_pos / lines_per_screen) + 1;
    }

    int get_bounded_scroll_position(int scroll_pos)
    {
        return std::max(
            0,
            std::min(
                scroll_pos,
                static_cast<int>(lines.size()) - num_text_display_lines()
            )
        );
    }

    TextViewState(std::vector<std::string> lines, TTF_Font *font)
        : font(font),
          line_height(detect_line_height(font)),
          num_display_lines((SCREEN_HEIGHT - line_padding) / (line_height + line_padding)),
          lines(lines),
          line_scroll_throttle(250, 50),
          page_scroll_throttle(250, 150)
    {
    }
};

TextView::TextView(std::vector<std::string> lines, TTF_Font *font)
    : state(std::make_unique<TextViewState>(lines, font))
{
}

TextView::~TextView()
{
}

bool TextView::render(SDL_Surface *dest_surface)
{
    TTF_Font *font = state->font;
    const int line_height = state->line_height;
    const int line_padding = state->line_padding;

    if (!state->needs_render)
    {
        return false;
    }
    state->needs_render = false;

    const SDL_PixelFormat *pixel_format = dest_surface->format;

    // TODO
    SDL_Color fg_color = {188, 182, 128, 0}; 
    SDL_Color bg_color = {0, 0, 0, 0};
    SDL_Color fg_title_color = {188 / 2, 182 / 2, 128 / 2, 0};
    uint32_t rect_bg_color = SDL_MapRGB(pixel_format, 0, 0, 0);

    // Clear screen
    SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_FillRect(dest_surface, &rect, rect_bg_color);

    Sint16 y = line_padding;

    int num_text_display_lines = state->num_text_display_lines();
    for (int i = 0; i < num_text_display_lines; ++i)
    {
        int global_i = i + state->scroll_pos;
        if (global_i >= static_cast<int>(state->lines.size()))
        {
            break;
        }

        SDL_Rect dest_rect = {0, y, 0, 0};
        SDL_Surface *surface = TTF_RenderUTF8_Shaded(font, state->lines[global_i].c_str(), fg_color, bg_color);
        SDL_BlitSurface(surface, nullptr, dest_surface, &dest_rect);
        SDL_FreeSurface(surface);

        y += line_height + line_padding;
    }

    if (state->show_title_bar)
    {
        // y = SCREEN_HEIGHT - line_height - line_padding;
        y = 2 * line_padding + (line_height + line_padding) * num_text_display_lines;

        SDL_Rect dest_rect = {0, y, 0, 0};
        SDL_Rect title_crop_rect = {0, 0, 0, (Uint16)line_height};

        {
            char page_number[32];
            snprintf(page_number, sizeof(page_number), " %d/%d", state->current_pages(), state->num_pages());

            SDL_Surface *page_surface = TTF_RenderUTF8_Shaded(font, page_number, fg_title_color, bg_color);

            dest_rect.x = SCREEN_WIDTH - page_surface->w - line_padding;
            title_crop_rect.w = SCREEN_WIDTH - line_padding * 2 - page_surface->w;

            SDL_BlitSurface(page_surface, nullptr, dest_surface, &dest_rect);
            SDL_FreeSurface(page_surface);
        }

        if (state->title.size() > 0)
        {
            dest_rect.x = 0;
            SDL_Surface *surface = TTF_RenderUTF8_Shaded(font, state->title.c_str(), fg_title_color, bg_color);
            SDL_BlitSurface(surface, &title_crop_rect, dest_surface, &dest_rect);
            SDL_FreeSurface(surface);
        }
    }

    return true;
}

void TextView::scroll(int num_lines, bool is_held_key)
{
    int new_scroll_pos = state->get_bounded_scroll_position(state->scroll_pos + num_lines);

    if (new_scroll_pos != state->scroll_pos)
    {
        state->scroll_pos = new_scroll_pos;
        state->needs_render = true;
    }
    else if (!is_held_key)
    {
        if (num_lines > 0)
        {
            if (state->on_resist_down)
            {
                state->on_resist_down();
            }
        }
        else
        {
            if (state->on_resist_up)
            {
                state->on_resist_up();
            }
        }
    }
}

void TextView::on_keypress(SDLKey key, bool is_held_key)
{
    switch (key) {
        case SW_BTN_UP:
            scroll(-1, is_held_key);
            break;
        case SW_BTN_DOWN:
            scroll(1, is_held_key);
            break;
        case SW_BTN_LEFT:
        case SW_BTN_L1:
            scroll(-state->num_text_display_lines(), is_held_key);
            break;
        case SW_BTN_RIGHT:
        case SW_BTN_R1:
            scroll(state->num_text_display_lines(), is_held_key);
            break;
        default:
            break;
    }
}

void TextView::on_keypress(SDLKey key)
{
    bool is_heldkey = false;
    on_keypress(key, is_heldkey);
}

void TextView::on_keyheld(SDLKey key, uint32_t held_time_ms)
{
    switch (key) {
        case SW_BTN_UP:
        case SW_BTN_DOWN:
            if (state->line_scroll_throttle(held_time_ms))
            {
                bool is_heldkey = true;
                on_keypress(key, is_heldkey);
            }
            break;
        case SW_BTN_LEFT:
        case SW_BTN_RIGHT:
        case SW_BTN_L1:
        case SW_BTN_R1:
            if (state->page_scroll_throttle(held_time_ms))
            {
                bool is_heldkey = true;
                on_keypress(key, is_heldkey);
            }
            break;
        default:
            break;
    }
}

bool TextView::is_done()
{
    return false;
}

void TextView::on_lose_focus()
{
    state->needs_render = true;
}

uint32_t TextView::get_line_number() const
{
    return state->scroll_pos;
}

void TextView::set_line_number(uint32_t line_number)
{
    if (line_number < state->lines.size())
    {
        state->scroll_pos = state->get_bounded_scroll_position(line_number);
        state->needs_render = true;
    }
}

void TextView::set_show_title_bar(bool enabled)
{
    if (enabled != state->show_title_bar)
    {
        state->show_title_bar = enabled;
        state->needs_render = true;
        state->scroll_pos = state->get_bounded_scroll_position(state->scroll_pos);
    }
}

void TextView::set_title(std::string title)
{
    if (title != state->title)
    {
        state->title = title;
        state->needs_render = true;
    }
}

void TextView::set_on_resist_up(std::function<void()> callback)
{
    state->on_resist_up = callback;
}

void TextView::set_on_resist_down(std::function<void()> callback)
{
    state->on_resist_down = callback;
}
