#include "./color_theme_def.h"
#include "./state_store.h"
#include "./system_styling.h"
#include "./views/file_selector.h"
#include "./views/reader_view.h"
#include "./view_stack.h"
#include "./views/text_view_styling.h"
#include "sys/keymap.h"
#include "sys/screen.h"
#include "sys/timing.h"
#include "util/fps_limiter.h"
#include "util/held_key_tracker.h"
#include "util/sdl_pointer.h"

#include <libxml/parser.h>
#include <SDL/SDL.h>

#include <iostream>

namespace
{

constexpr const char *SETTINGS_KEY_COLOR_THEME = "color_theme";
constexpr const char *SETTINGS_KEY_SHOW_TITLE_BAR = "show_title_bar";

bool settings_get_show_title_bar(const StateStore &state_store)
{
    return state_store.get_setting(SETTINGS_KEY_SHOW_TITLE_BAR).value_or("true") == "true";
}

void settings_set_show_title_bar(StateStore &state_store, bool show_title_bar)
{
    state_store.set_setting(SETTINGS_KEY_SHOW_TITLE_BAR, show_title_bar ? "true" : "false");
}

std::string settings_get_color_theme(const StateStore &state_store)
{
    return state_store.get_setting(SETTINGS_KEY_COLOR_THEME).value_or(get_next_theme(""));
}

void settings_set_color_theme(StateStore &state_store, const std::string &color_theme)
{
    state_store.set_setting(SETTINGS_KEY_COLOR_THEME, color_theme);
}

void initialize_views(ViewStack &view_stack, StateStore &state_store, SystemStyling &sys_styling, TextViewStyling &text_view_styling, TTF_Font *mono_font)
{
    auto browse_path = state_store.get_current_browse_path().value_or(std::filesystem::current_path() / "");
    std::shared_ptr<FileSelector> fs = std::make_shared<FileSelector>(
        browse_path,
        sys_styling,
        mono_font
    );

    auto load_book = [&view_stack, &state_store, &sys_styling, &text_view_styling](std::filesystem::path path) {
        if (path.extension() != ".epub")
        {
            return;
        }
        state_store.set_current_book_path(path);

        auto reader_view = std::make_shared<ReaderView>(
            path,
            state_store.get_book_address(path).value_or(0),
            sys_styling,
            text_view_styling,
            view_stack
        );

        reader_view->set_on_change_address([&state_store, path_str=path.string()](const DocAddr &addr) {
            state_store.set_book_address(path_str, addr);
        });
        reader_view->set_on_quit_requested([&state_store]() {
            state_store.remove_current_book_path();
        });

        view_stack.push(reader_view);
    };

    fs->set_on_file_selected(load_book);
    fs->set_on_file_focus([&state_store](std::string path) {
        state_store.set_current_browse_path(path);
    });

    view_stack.push(fs);

    if (state_store.get_current_book_path())
    {
        load_book(state_store.get_current_book_path().value());
    }
}

} // namespace

int main(int, char *[])
{
    // SDL Init
    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    TTF_Init();

    // Stores
    auto base_dir = std::filesystem::current_path() / ".state";
    StateStore state_store(base_dir);

    // System styling
    int font_size = 18;
    std::string mono_font_path = "fonts/DejaVuSansMono.ttf";
    ttf_font_unique_ptr mono_font(
        TTF_OpenFont(mono_font_path.c_str(), font_size)
    );
    if (!mono_font)
    {
        std::cerr << "Failed to load font" << std::endl;
        return 1;
    }

    SystemStyling sys_styling(
        settings_get_color_theme(state_store)
    );
    sys_styling.subscribe_to_changes([&state_store, &sys_styling]() {
        // Persist changes to system styling
        settings_set_color_theme(state_store, sys_styling.get_color_theme());
    });

    // Text Styling
    TextViewStyling text_view_styling(
        "fonts/DejaVuSans.ttf",
        font_size,
        settings_get_show_title_bar(state_store)
    );
    text_view_styling.subscribe_to_changes([&text_view_styling, &state_store]() {
        // Persist changes to text view styling
        settings_set_show_title_bar(state_store, text_view_styling.get_show_title_bar());
    });
    if (!text_view_styling.get_loaded_font())
    {
        std::cerr << "Failed to load font" << std::endl;
        return 1;
    }

    // Views
    ViewStack view_stack;
    initialize_views(view_stack, state_store, sys_styling, text_view_styling, mono_font.get());

    // Track held keys
    HeldKeyTracker held_key_tracker(
        {
            SW_BTN_UP,
            SW_BTN_DOWN,
            SW_BTN_LEFT,
            SW_BTN_RIGHT,
            SW_BTN_L1,
            SW_BTN_R1
        }
    );

    auto key_held_callback = [&view_stack](SDLKey key, uint32_t held_ms) {
        view_stack.on_keyheld(key, held_ms);
    };

    // Timing
    FPSLimiter limit_fps(TARGET_FPS);
    const uint32_t avg_loop_time = 1000 / TARGET_FPS;

    // Surfaces
    SDL_Surface *video = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE);
    SDL_Surface *screen = SDL_CreateRGBSurface(SDL_HWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
    view_stack.render(screen, true);

    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    bool quit = false;
    while (!quit)
    {
        bool needs_render = false;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    {
                        SDLKey key = event.key.keysym.sym;
                        if (key == SW_BTN_MENU)
                        {
                            quit = true;
                        }
                        else if (key == SW_BTN_X)
                        {
                            sys_styling.set_color_theme(
                                get_next_theme(sys_styling.get_color_theme())
                            );
                            needs_render = true;
                        }
                        else
                        {
                            view_stack.on_keypress(key);
                            needs_render = true;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        held_key_tracker.accumulate(avg_loop_time); // Pretend perfect loop timing for event firing consistency
        needs_render = held_key_tracker.for_each_held_key(key_held_callback) || needs_render;

        if (needs_render)
        {
            bool force_render = view_stack.pop_completed_views();

            if (view_stack.is_done())
            {
                quit = true;
            }

            if (view_stack.render(screen, force_render))
            {
                SDL_BlitSurface(screen, NULL, video, NULL);
                SDL_Flip(video);
            }
        }

        if (!quit)
        {
            limit_fps();
        }
    }

    view_stack.shutdown();
    state_store.flush();

    SDL_FreeSurface(screen);
    SDL_Quit();
    xmlCleanupParser();
    
    return 0;
}
