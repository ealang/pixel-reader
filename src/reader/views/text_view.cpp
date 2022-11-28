#include "./text_view.h"
#include "reader/text_wrap.h"
#include "reader/sdl_utils.h"

#include "sys/keymap.h"
#include "sys/screen.h"
#include "sys/timer.h"

#include <vector>

struct TextViewState
{
    bool needs_render = true;
    std::vector<std::string> lines;
    int scroll_pos = 0;
    int num_display_lines = 0;

    std::function<void()> on_resist_up;
    std::function<void()> on_resist_down;

    TextViewState(std::vector<std::string> lines) : lines(lines) {}
};

TextView::TextView(std::vector<std::string> lines, TTF_Font *font)
    : state(std::make_unique<TextViewState>(lines)),
      font(font),
      line_height(detect_line_height(font))
{

    state->num_display_lines = (SCREEN_HEIGHT - line_padding) / (line_height + line_padding);
}

TextView::~TextView()
{
}

bool TextView::render(SDL_Surface *dest_surface)
{
    if (!state->needs_render)
    {
        return false;
    }
    state->needs_render = false;

    const SDL_PixelFormat *pixel_format = dest_surface->format;
    SDL_Color fg_color = {188, 182, 128, 0};
    SDL_Color bg_color = {0, 0, 0, 0};
    uint32_t rect_bg_color = SDL_MapRGB(pixel_format, 0, 0, 0);

    // Clear screen
    SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_FillRect(dest_surface, &rect, rect_bg_color);

    Sint16 y = line_padding;
    for (int i = 0; i < state->num_display_lines; ++i)
    {
        int global_i = i + state->scroll_pos;
        if (global_i >= static_cast<int>(state->lines.size()))
        {
            break;
        }

        SDL_Rect rectMessage = {0, y, 0, 0};
        SDL_Surface *message = TTF_RenderUTF8_Shaded(font, state->lines[global_i].c_str(), fg_color, bg_color);
        SDL_BlitSurface(message, NULL, dest_surface, &rectMessage);
        SDL_FreeSurface(message);

        y += line_height + line_padding;
    }

    return true;
}

static int bounded_scroll(int scroll_pos, int num_display_lines, int num_lines)
{
    return std::max(
        0, 
        std::min(
            scroll_pos,
            num_lines - num_display_lines
        )
    );
}

void TextView::scroll_up(int num_lines)
{
    int new_scroll_pos = bounded_scroll(
        state->scroll_pos - num_lines,
        state->num_display_lines,
        state->lines.size()
    );

    if (new_scroll_pos != state->scroll_pos)
    {
        state->scroll_pos = new_scroll_pos;
        state->needs_render = true;
    }
    else if (state->on_resist_up)
    {
        state->on_resist_up();
    }
}

void TextView::scroll_down(int num_lines)
{
    int new_scroll_pos = bounded_scroll(
        state->scroll_pos + num_lines,
        state->num_display_lines,
        state->lines.size()
    );
    if (new_scroll_pos != state->scroll_pos)
    {
        state->scroll_pos = new_scroll_pos;
        state->needs_render = true;
    }
    else if (state->on_resist_down)
    {
        state->on_resist_down();
    }
}

bool TextView::on_keypress(SDLKey key)
{
    switch (key) {
        case SW_BTN_UP:
            scroll_up(1);
            break;
        case SW_BTN_DOWN:
            scroll_down(1);
            break;
        case SW_BTN_LEFT:
            scroll_up(state->num_display_lines);
            break;
        case SW_BTN_RIGHT:
            scroll_down(state->num_display_lines);
            break;
        default:
            break;
    }

    return true;
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
        state->scroll_pos = bounded_scroll(
            line_number,
            state->num_display_lines,
            state->lines.size()
        );
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
