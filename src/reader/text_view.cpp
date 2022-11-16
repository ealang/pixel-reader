#include "sys/screen.h"
#include "sys/keymap.h"
#include "./text_view.h"
#include "./text_wrap.h"

#include "sys/timer.h"

#include <vector>

struct TextViewState
{
    bool needs_render = true;
    std::vector<std::string> lines;
    int scroll_pos = 0;
    int num_display_lines = 0;

    TextViewState(std::vector<std::string> lines) : lines(lines) {}
};

static int detect_line_height(TTF_Font *font)
{
    int w, h;
    if (TTF_SizeUTF8(font, "A", &w, &h) == 0)
    {
        return h;
    }
    return 24;
}

TextView::TextView(std::vector<std::string> lines, TTF_Font *font, int line_padding)
    : state(std::make_unique<TextViewState>(lines)),
      font(font),
      line_height(detect_line_height(font)),
      line_padding(line_padding)
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

bool TextView::on_keypress(SDLKey key)
{
    switch (key) {
        case SW_BTN_UP:
            if (state->scroll_pos > 0)
            {
                state->scroll_pos--;
                state->needs_render = true;
            }
            break;
        case SW_BTN_DOWN:
            if (state->scroll_pos < static_cast<int>(state->lines.size()) - state->num_display_lines)
            {
                state->scroll_pos++;
                state->needs_render = true;
            }
            break;
        case SW_BTN_LEFT:
            if (state->scroll_pos >= 0)
            {
                state->scroll_pos = std::max(0, state->scroll_pos - state->num_display_lines);
                state->needs_render = true;
            }
            break;
        case SW_BTN_RIGHT:
            if (state->scroll_pos < static_cast<int>(state->lines.size()) - state->num_display_lines)
            {
                state->scroll_pos = std::min(
                    static_cast<int>(state->lines.size()) - state->num_display_lines,
                    state->scroll_pos + state->num_display_lines
                );
                state->needs_render = true;
            }
            break;
        default:
            _is_done = true;
            break;
    }

    return true;
}

bool TextView::is_done()
{
    return _is_done;
}
