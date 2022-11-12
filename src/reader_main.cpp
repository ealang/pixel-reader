#include <iostream>
#include <algorithm>
#include "filesystem.h"
#include "filesystem_path.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <set>

// from Onion/src/common/system/keymap_sw.h
#define SW_BTN_UP       SDLK_UP
#define SW_BTN_DOWN     SDLK_DOWN
#define SW_BTN_LEFT     SDLK_LEFT
#define SW_BTN_RIGHT    SDLK_RIGHT
#define SW_BTN_A        SDLK_SPACE
#define SW_BTN_B        SDLK_LCTRL
#define SW_BTN_X        SDLK_LSHIFT
#define SW_BTN_Y        SDLK_LALT
#define SW_BTN_L1       SDLK_e
#define SW_BTN_R1       SDLK_t
#define SW_BTN_L2       SDLK_TAB
#define SW_BTN_R2       SDLK_BACKSPACE
#define SW_BTN_SELECT   SDLK_RCTRL
#define SW_BTN_START    SDLK_RETURN
#define SW_BTN_MENU     SDLK_ESCAPE
#define SW_BTN_POWER    SDLK_FIRST

/*
SDL_Surface *createSurface(int width, int height)
{
    return SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, screen.surface->format->BitsPerPixel, screen.surface->format->Rmask, screen.surface->format->Gmask, screen.surface->format->Bmask, screen.surface->format->Amask);
}

SDL_Surface *createImage(const int p_width, const int p_height, const Uint32 p_color)
{
    SDL_Surface *l_ret = createSurface(p_width, p_height);
    if (l_ret == NULL)
        std::cerr << "createImage: " << SDL_GetError() << std::endl;
    // Fill image with the given color
    SDL_FillRect(l_ret, NULL, p_color);
    return l_ret;
}
*/

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

class FileSelector
{
    std::string path;
    TTF_Font *font;  // TODO: shared pointer?
    int font_ptsize;
    int cursor_pos;
    int scroll_pos;
    int line_padding = 6;
    int num_lines;

    std::vector<FSEntry> path_entries;

    std::string selected_file;

    void refresh_path_entries()
    {
        std::cout << path << std::endl;
        path_entries = directory_listing(path);
        if (path_entries.size() && path != "/")
        {
            path_entries.insert(path_entries.begin(), FSEntry::directory(".."));
        }
    }

    void move_up()
    {
        cursor_pos = std::max(cursor_pos - 1, 0);
        if (cursor_pos < scroll_pos)
        {
            scroll_pos = cursor_pos;
        }
    }

    void move_down()
    {
        int num_entries = (int)path_entries.size();
        if (num_entries)
        {
            cursor_pos = std::min(cursor_pos + 1, num_entries - 1);
            if (cursor_pos >= scroll_pos + num_lines)
            {
                scroll_pos = cursor_pos - num_lines + 1;
            }
        }
    }

    void select_entry()
    {
        int num_entries = (int)path_entries.size();
        if (num_entries)
        {
            const FSEntry &entry = path_entries[cursor_pos];
            if (entry.is_dir)
            {
                std::string select_name;
                if (entry.name == "..")
                {
                    select_name = fs_path_split_dir(path).second;

                    path = fs_path_parent(path);
                    if (path.empty())
                    {
                        path = "/";
                    }
                }
                else
                {
                    path = fs_path_join(path, entry.name);
                }

                cursor_pos = 0;
                scroll_pos = 0;

                refresh_path_entries();
                if (!select_name.empty())
                {
                    for (int i = 0; i < (int)path_entries.size(); ++i)
                    {
                        if (path_entries[i].name == select_name && path_entries[i].is_dir)
                        {
                            cursor_pos = i;
                            scroll_pos = std::max(0, i - (num_lines / 2));
                            break;
                        }
                    }
                }
                else
                {
                    move_down();
                }
                // TODO directory changed, out of bounds
            }
            else
            {
                selected_file = fs_path_join(path, entry.name);
            }
        }
    }

public:
    FileSelector(const std::string &path, TTF_Font *font, uint32_t font_ptsize)
        : path(path), font(font), font_ptsize(font_ptsize), cursor_pos(0), scroll_pos(0)
    {
        num_lines = (SCREEN_HEIGHT - 2 * line_padding) / (font_ptsize + 2 * line_padding);
        refresh_path_entries();
        move_down();
    }

    bool file_is_selected() const
    {
        return selected_file.size() > 0;
    }

    std::string get_selected_file()
    {
        return selected_file;
    }

    void render(SDL_Surface *dest_surface)
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
        for (int i = 0; i < num_lines; ++i)
        {
            int global_i = i + scroll_pos;
            if (global_i >= (int)path_entries.size())
            {
                break;
            }

            const FSEntry &entry = path_entries[global_i];

            bool is_highlighted = (global_i == cursor_pos);

            // Draw hightlight
            if (is_highlighted)
            {
                SDL_Rect rect = {0, y, SCREEN_WIDTH, (Uint16)(font_ptsize + 2 * line_padding)};
                SDL_FillRect(dest_surface, &rect, rect_highlight_color);
            }

            // Draw text
            {
                SDL_Rect rectMessage = {x, (Sint16)(y + line_padding), 0, 0};
                std::string name = (entry.is_dir ? "D " : "F ") + entry.name;
                SDL_Surface *message = TTF_RenderUTF8_Shaded(font, name.c_str(), fg_color, is_highlighted ? highlight_color : bg_color);
                SDL_BlitSurface(message, NULL, dest_surface, &rectMessage);
                SDL_FreeSurface(message);
            }

            y += font_ptsize + 2 * line_padding;
        }
    }

    bool on_keypress(SDLKey key)
    {
        switch (key) {
            case SW_BTN_DOWN:
                move_down();
                return true;
            case SW_BTN_UP:
                move_up();
                return true;
            case SW_BTN_A:
                select_entry();
                return true;
            default:
                return false;
        }
    }
};

int main () {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    TTF_Init();

    SDL_Surface *video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    SDL_Surface *screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    int font_size = 24;
    TTF_Font *font = TTF_OpenFont(FONT_PATH, font_size);

    if (!font) {
        std::cerr << "TTF_OpenFont: " << TTF_GetError() << std::endl;
        return 1;
    }

    int w, h;
    if (TTF_SizeUTF8(font, "A", &w, &h) == 0)
    {
        std::cout << "Font size: " << w << "x" << h << std::endl;
    }

    FileSelector selector(get_cwd(), font, font_size);

    selector.render(screen);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    bool quit = false;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            bool rerender = false;
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    if (selector.on_keypress(event.key.keysym.sym))
                    {
                        rerender = true;

                        if (selector.file_is_selected())
                        {
                            std::cout << "Selected file: " << selector.get_selected_file() << std::endl;
                            quit = true;
                        }
                    }
                    else
                    {
                        switch (event.key.keysym.sym) {
                            case SW_BTN_LEFT:
                                break;
                            case SW_BTN_RIGHT:
                                break;
                            case SW_BTN_A:
                                quit = true;
                                break;
                            case SW_BTN_MENU:
                                quit = true;
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }

            if (rerender)
            {
                selector.render(screen);
                SDL_BlitSurface(screen, NULL, video, NULL);
                SDL_Flip(video);
            }
        }
    }

    TTF_CloseFont(font);
    SDL_FreeSurface(screen);
    SDL_Quit();
    
    return 0;
}
