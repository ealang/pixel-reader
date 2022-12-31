#include "./config.h"
#include "./font_catalog.h"
#include "./settings_store.h"
#include "./state_store.h"
#include "./system_styling.h"
#include "./color_theme_def.h"
#include "./view_stack.h"
#include "./views/file_selector.h"
#include "./views/reader_view.h"
#include "./views/settings_view.h"
#include "./views/token_view/token_view_styling.h"
#include "epub/epub_reader.h"
#include "sys/keymap.h"
#include "sys/screen.h"
#include "util/fps_limiter.h"
#include "util/held_key_tracker.h"
#include "util/sdl_font_cache.h"
#include "util/str_utils.h"
#include "util/timer.h"

#include <libxml/parser.h>
#include <SDL/SDL.h>

#include <csignal>
#include <iostream>

namespace
{

void initialize_views(ViewStack &view_stack, StateStore &state_store, SystemStyling &sys_styling, TokenViewStyling &token_view_styling)
{
    auto browse_path = state_store.get_current_browse_path().value_or(std::filesystem::current_path() / "");
    std::shared_ptr<FileSelector> fs = std::make_shared<FileSelector>(
        browse_path,
        sys_styling
    );

    auto load_book = [&view_stack, &state_store, &sys_styling, &token_view_styling](std::filesystem::path path) {
        if (to_lower(path.extension()) != ".epub" || !std::filesystem::exists(path))
        {
            return;
        }
        state_store.set_current_book_path(path);

        auto reader_view = std::make_shared<ReaderView>(
            path,
            state_store.get_book_address(path).value_or(0),
            sys_styling,
            token_view_styling,
            view_stack
        );

        reader_view->set_on_change_address([&state_store, path_str=path.string()](DocAddr addr) {
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

bool quit = false;

void signal_handler(int)
{
    quit = true;
}

} // namespace

int main(int, char *[])
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // SDL Init
    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    TTF_Init();

    // Surfaces
    SDL_Surface *video = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE);
    SDL_Surface *screen = SDL_CreateRGBSurface(SDL_HWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
    set_render_surface_format(screen->format);

    // Store
    StateStore state_store(".state");

    // Preload & check fonts
    auto init_font_name = get_valid_font_name(settings_get_font_name(state_store).value_or(DEFAULT_FONT_NAME));
    auto init_font_size = settings_get_font_size(state_store).value_or(DEFAULT_FONT_SIZE);
    if (
        !cached_load_font(SYSTEM_FONT, init_font_size, FontLoadErrorOpt::NoThrow) ||
        !cached_load_font(init_font_name, init_font_size, FontLoadErrorOpt::NoThrow)
    )
    {
        std::cerr << "Failed to load one or more fonts" << std::endl;
        return 1;
    }

    // System styling
    SystemStyling sys_styling(
        init_font_name,
        init_font_size,
        get_valid_theme(settings_get_color_theme(state_store).value_or(DEFAULT_COLOR_THEME))
    );
    sys_styling.subscribe_to_changes([&state_store, &sys_styling]() {
        // Persist changes
        settings_set_color_theme(state_store, sys_styling.get_color_theme());
        settings_set_font_name(state_store, sys_styling.get_font_name());
        settings_set_font_size(state_store, sys_styling.get_font_size());
    });

    // Text Styling
    TokenViewStyling token_view_styling(settings_get_show_title_bar(state_store).value_or(DEFAULT_SHOW_PROGRESS));
    token_view_styling.subscribe_to_changes([&token_view_styling, &state_store]() {
        // Persist changes
        settings_set_show_title_bar(state_store, token_view_styling.get_show_title_bar());
    });

    // Setup views
    ViewStack view_stack;
    initialize_views(view_stack, state_store, sys_styling, token_view_styling);

    std::shared_ptr<SettingsView> settings_view = std::make_shared<SettingsView>(
        sys_styling,
        SYSTEM_FONT
    );

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
    Timer idle_timer;
    FPSLimiter limit_fps(TARGET_FPS);
    const uint32_t avg_loop_time = 1000 / TARGET_FPS;

    // Initial render
    view_stack.render(screen, true);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

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
                        idle_timer.reset();

                        SDLKey key = event.key.keysym.sym;
                        if (key == SW_BTN_MENU)
                        {
                            quit = true;
                        }
                        else if (key == SW_BTN_POWER)
                        {
                            state_store.flush();
                        }
                        else if (key == SW_BTN_X)
                        {
                            if (view_stack.top_view() != settings_view)
                            {
                                settings_view->unterminate();
                                view_stack.push(settings_view);
                            }
                            else
                            {
                                settings_view->terminate();
                            }
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

        if (idle_timer.elapsed_sec() >= IDLE_SAVE_TIME_SEC)
        {
            // Make sure state is saved in case device auto-powers down. Don't seem
            // to get a signal on miyoo mini when this happens.
            state_store.flush();
            idle_timer.reset();
        }
    }

    view_stack.shutdown();
    state_store.flush();

    SDL_FreeSurface(screen);
    SDL_Quit();
    xmlCleanupParser();
    
    return 0;
}
