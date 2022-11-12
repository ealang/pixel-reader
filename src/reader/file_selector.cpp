#include "./file_selector.h"

#include "sys/filesystem.h"
#include "sys/filesystem_path.h"
#include "sys/screen.h"
#include "sys/keymap.h"

#include <iostream>
#include <vector>

struct FSState
{
    // display
    TTF_Font *font;
    int line_height;
    int line_padding;
    int num_display_lines;
    bool needs_render = true;

    // file state
    std::string path;
    std::string selected_file;
    std::vector<FSEntry> path_entries;

    // selection state
    int cursor_pos = 0;
    int scroll_pos = 0;

    FSState(const std::string &path, TTF_Font *font, int line_height, int line_padding)
        : font(font),
          line_height(line_height),
          line_padding(line_padding),
          num_display_lines((SCREEN_HEIGHT - line_padding) / (line_height + line_padding)),
          path(path)
    {
    }
};

static void refresh_path_entries(FSState *s)
{
    std::cerr << s->path << std::endl;

    s->path_entries = directory_listing(s->path);
    if (s->path != "/")
    {
        s->path_entries.insert(s->path_entries.begin(), FSEntry::directory(".."));
    }
}

static void on_move_down(FSState *s)
{
    int num_entries = s->path_entries.size();
    if (s->cursor_pos < num_entries - 1)
    {
        s->cursor_pos++;
        if (s->cursor_pos >= s->scroll_pos + s->num_display_lines)
        {
            s->scroll_pos = s->cursor_pos - s->num_display_lines + 1;
        }
        s->needs_render = true;
    }
}

static void on_move_up(FSState *s)
{
    s->cursor_pos = std::max(s->cursor_pos - 1, 0);
    s->scroll_pos = std::min(s->scroll_pos, s->cursor_pos);
    s->needs_render = true;
}

static void on_select_entry(FSState *s)
{
    if (s->path_entries.empty())
    {
        return;
    }

    const FSEntry &entry = s->path_entries[s->cursor_pos];
    if (entry.is_dir)
    {
        std::string highlight_name;

        if (entry.name == "..")
        {
            highlight_name = fs_path_split_dir(s->path).second;

            // update path
            s->path = fs_path_parent(s->path);
            if (s->path.empty())
            {
                s->path = "/"; // TODO: add logic to fs_path_parents
            }
        }
        else
        {
            // go down a directory
            s->path = fs_path_join(s->path, entry.name);
            s->cursor_pos = 1;
            s->scroll_pos = 0;
        }

        refresh_path_entries(s);
        if (!highlight_name.empty())
        {
            for (int i = 0; i < (int)s->path_entries.size(); ++i)
            {
                if (s->path_entries[i].name == highlight_name && s->path_entries[i].is_dir)
                {
                    s->cursor_pos = i;
                    s->scroll_pos = std::max(0, i - (s->num_display_lines / 2));
                    break;
                }
            }
        }

        s->cursor_pos = std::min(s->cursor_pos, (int)s->path_entries.size() - 1);
    }
    else
    {
        // selected a file
        s->selected_file = fs_path_join(s->path, entry.name);
    }

    s->needs_render = true;
}

static int detect_line_height(TTF_Font *font)
{
    int w, h;
    if (TTF_SizeUTF8(font, "A", &w, &h) == 0)
    {
        return h;
    }
    return 24;
}

FileSelector::FileSelector(const std::string &path, TTF_Font *font, int line_padding)
    : state(std::make_unique<FSState>(
          fs_path_make_absolute(path, get_cwd()),
          font,
          detect_line_height(font),
          line_padding
      ))
{
    refresh_path_entries(state.get());

}

bool FileSelector::file_is_selected() const
{
    return state->selected_file.size() > 0;
}

std::string FileSelector::get_selected_file() const
{
    return state->selected_file;
}

bool FileSelector::render(SDL_Surface *dest_surface) const
{
    if (!state->needs_render)
    {
        return false;
    }
    state->needs_render = false;

    const SDL_PixelFormat *pixel_format = dest_surface->format;

    SDL_Color fg_color = {255, 255, 255, 0};
    SDL_Color bg_color = {0, 0, 0, 0};
    SDL_Color highlight_color = {128, 0, 128, 0};

    uint32_t rect_bg_color = SDL_MapRGB(pixel_format, 0, 0, 0);
    uint32_t rect_highlight_color = SDL_MapRGB(pixel_format, 128, 0, 128);

    Sint16 x = state->line_padding;
    Sint16 y = state->line_padding;

    // Clear screen
    SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_FillRect(dest_surface, &rect, rect_bg_color);

    // Draw lines
    for (int i = 0; i < state->num_display_lines; ++i)
    {
        int global_i = i + state->scroll_pos;
        if (global_i >= (int)state->path_entries.size())
        {
            break;
        }

        const FSEntry &entry = state->path_entries[global_i];

        bool is_highlighted = (global_i == state->cursor_pos);

        // Draw hightlight
        if (is_highlighted)
        {
            SDL_Rect rect = {0, (Sint16)(y - state->line_padding / 2), SCREEN_WIDTH, (Uint16)(state->line_height + state->line_padding)};
            SDL_FillRect(dest_surface, &rect, rect_highlight_color);
        }

        // Draw text
        {
            SDL_Rect rectMessage = {x, y, 0, 0};
            std::string name = (entry.is_dir ? "D " : "F ") + entry.name;
            SDL_Surface *message = TTF_RenderUTF8_Shaded(state->font, name.c_str(), fg_color, is_highlighted ? highlight_color : bg_color);
            SDL_BlitSurface(message, NULL, dest_surface, &rectMessage);
            SDL_FreeSurface(message);
        }

        y += state->line_height + state->line_padding;
    }

    return true;
}

bool FileSelector::on_keypress(SDLKey key)
{
    switch (key) {
        case SW_BTN_DOWN:
            on_move_down(state.get());
            return true;
        case SW_BTN_UP:
            on_move_up(state.get());
            return true;
        case SW_BTN_A:
            on_select_entry(state.get());
            return true;
        default:
            return false;
    }
}

FileSelector::~FileSelector()
{
}
