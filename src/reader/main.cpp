#include "./config.h"
#include "./font_catalog.h"
#include "./settings_store.h"
#include "./shoulder_keymap.h"
#include "./state_store.h"
#include "./system_styling.h"
#include "./color_theme_def.h"
#include "./view_stack.h"
#include "./views/file_selector.h"
#include "./views/reader_bootstrap_view.h"
#include "./views/settings_view.h"
#include "./views/token_view/token_view_styling.h"
#include "filetypes/open_doc.h"
#include "sys/keymap.h"
#include "sys/screen.h"
#include "util/fps_limiter.h"
#include "util/held_key_tracker.h"
#include "util/key_value_file.h"
#include "util/math.h"
#include "util/sdl_font_cache.h"
#include "util/task_queue.h"
#include "util/timer.h"
#include "util/screen_rotation.h"
#include "extern/rotozoom/SDL_rotozoom.h"

#include <libxml/parser.h>
#include <SDL/SDL.h>

#include <csignal>
#include <iostream>

namespace
{

void initialize_views(
    ViewStack &view_stack,
    StateStore &state_store,
    SystemStyling &sys_styling,
    TokenViewStyling &token_view_styling,
    TaskQueue &task_queue,
    std::optional<std::filesystem::path> requested_book_path
)
{
    auto load_book = [&view_stack, &state_store, &sys_styling, &token_view_styling, &task_queue](std::filesystem::path path) {
        if (!std::filesystem::exists(path))
        {
            std::cerr << path << " does not exist" << std::endl;
            return;
        }
        if (!file_type_is_supported(path))
        {
            std::cerr << path << " filetype is not supported" << std::endl;
            return;
        }

        view_stack.push(
            std::make_shared<ReaderBootstrapView>(
                path,
                sys_styling,
                token_view_styling,
                view_stack,
                state_store,
                [&task_queue](task_func task){ task_queue.submit(task); }
            )
        );
    };

    if (requested_book_path)
    {
        load_book(*requested_book_path);
    }
    else
    {
        auto browse_path = state_store.get_current_browse_path().value_or(DEFAULT_BROWSE_PATH);
        std::shared_ptr<FileSelector> fs = std::make_shared<FileSelector>(
            browse_path,
            sys_styling
        );

        fs->set_on_file_selected(load_book);
        fs->set_on_file_focus([&state_store](std::string path) {
            state_store.set_current_browse_path(path);
        });
        fs->set_on_view_focus([&state_store]() {
            state_store.remove_current_book_path();
        });

        view_stack.push(fs);

        if (state_store.get_current_book_path())
        {
            load_book(state_store.get_current_book_path().value());
        }
    }
}

class SystemKeyChordTracker
{
    bool _menu_held = false;
    bool _select_held = false;

    bool _exit_on_menu_release = false;
    bool _exit_requested = false;

public:

    // Report keypress event. Return filtered key code.
    SDLKey on_keypress(SDLKey key)
    {
        // Block any other keys while special key is held
        SDLKey filtered_key = (_menu_held || _select_held) ? SDLK_UNKNOWN : key;

        if (key == SW_BTN_SELECT)
        {
            _select_held = true;
        }

        if (key == SW_BTN_MENU)
        {
            _menu_held = true;
            _exit_on_menu_release = true;
        }
        else
        {
            // Cancel app exit if a menu chord was used (e.g. change brightness)
            _exit_on_menu_release = false;
        }

        return filtered_key;
    }

    // Report keyrelease event.
    void on_keyrelease(SDLKey key)
    {
        if (key == SW_BTN_MENU)
        {
            _menu_held = false;

            if (_exit_on_menu_release)
            {
                _exit_requested = true;
            }
        }
        else if (key == SW_BTN_SELECT)
        {
            _select_held = false;
        }
    }

    bool exit_requested() const
    {
        return _exit_requested;
    }
};

bool quit = false;

void signal_handler(int)
{
    quit = true;
}

const char *CONFIG_KEY_STORE_PATH = "store_path";

std::unordered_map<std::string, std::string> load_config_with_defaults()
{
    auto config = load_key_value(CONFIG_FILE_PATH);
    config.try_emplace(CONFIG_KEY_STORE_PATH, FALLBACK_STORE_PATH);
    return config;
}

// Current rotation setting (default: no rotation)
ScreenRotation current_rotation = ROTATION_NONE;

// Apply screen rotation transform to the surface
void apply_screen_rotation(SDL_Surface* src, SDL_Surface* dest) {
    if (current_rotation == ROTATION_NONE) {
        // No rotation, just do a normal blit
        SDL_BlitSurface(src, NULL, dest, NULL);
        return;
    }
    
    // Create a temporary rotated surface
    SDL_Surface* rotated = rotozoomSurface(src, current_rotation, 1.0, SMOOTHING_OFF);
    if (!rotated) {
        // Fallback to normal blit if rotation fails
        SDL_BlitSurface(src, NULL, dest, NULL);
        return;
    }
    
    // Calculate destination position to center the rotated image
    SDL_Rect dest_rect;

    std::uint16_t dest_x = 0;
    std::uint16_t dest_y = 0;

    // auto dest_x = (dest->w - rotated->w) / 2;
    // auto dest_y = (dest->h - rotated->h) / 2;
    if (current_rotation == ROTATION_90) {
        dest_x = 0;
        dest_y = -160;
    }
    else if (current_rotation == ROTATION_180) {
        dest_x = 0;
        dest_y = -160;
    }
    else if (current_rotation == ROTATION_270) {
        dest_x = 0;
        dest_y = 0;
    }

    // auto dest_x = 0;
    // auto dest_y = -160;
    // std::cout << "dest_x: " << dest_x << ", dest_y: " << dest_y << std::endl;

    dest_rect.x = dest_x;
    dest_rect.y = dest_y;
    
    // Blit the rotated surface to the destination
    SDL_BlitSurface(rotated, NULL, dest, &dest_rect);
    
    // Free the temporary rotated surface
    SDL_FreeSurface(rotated);
}

} // namespace

int main(int argc, char **argv)
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (char* env_screen_width = SDL_getenv("SCREEN_WIDTH")) {
        int new_width = atoi(env_screen_width);
        if (100 < new_width && new_width < 4096)
            SCREEN_WIDTH = static_cast<unsigned int>(new_width);
    }

    if (char* env_screen_height = SDL_getenv("SCREEN_HEIGHT")) {
        int new_height = atoi(env_screen_height);
        if (100 < new_height && new_height < 4096)
            SCREEN_HEIGHT = static_cast<unsigned int>(new_height);
    }

    std::cout << "Screen Size: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << std::endl;

    // SDL Init
    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    TTF_Init();

    // Surfaces
    SDL_Surface *video = SDL_SetVideoMode(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        32,
        SDL_HWSURFACE
    );
    SDL_Surface *screen = SDL_CreateRGBSurface(
        SDL_HWSURFACE,
        std::max(SCREEN_WIDTH, SCREEN_HEIGHT),
        std::max(SCREEN_WIDTH, SCREEN_HEIGHT),
        32, 0, 0, 0, 0
    );
    set_render_surface_format(screen->format);

    auto config = load_config_with_defaults();
    StateStore state_store(config[CONFIG_KEY_STORE_PATH]);

    // Preload & check fonts
    auto init_font_name = get_valid_font_name(settings_get_font_name(state_store).value_or(DEFAULT_FONT_NAME));
    auto init_font_size = bound(settings_get_font_size(state_store).value_or(DEFAULT_FONT_SIZE), MIN_FONT_SIZE, MAX_FONT_SIZE);
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
        get_valid_theme(settings_get_color_theme(state_store).value_or(DEFAULT_COLOR_THEME)),
        get_valid_shoulder_keymap(settings_get_shoulder_keymap(state_store).value_or(DEFAULT_SHOULDER_KEYMAP))
    );
    sys_styling.subscribe_to_changes([&state_store, &sys_styling](SystemStyling::ChangeId) {
        // Persist changes
        settings_set_color_theme(state_store, sys_styling.get_color_theme());
        settings_set_font_name(state_store, sys_styling.get_font_name());
        settings_set_font_size(state_store, sys_styling.get_font_size());
        settings_set_shoulder_keymap(state_store, sys_styling.get_shoulder_keymap());
    });

    // Text Styling
    TokenViewStyling token_view_styling(
        settings_get_show_title_bar(state_store).value_or(DEFAULT_SHOW_PROGRESS),
        settings_get_progress_reporting(state_store).value_or(DEFAULT_PROGRESS_REPORTING)
    );
    token_view_styling.subscribe_to_changes([&token_view_styling, &state_store]() {
        // Persist changes
        settings_set_show_title_bar(state_store, token_view_styling.get_show_title_bar());
        settings_set_progress_reporting(state_store, token_view_styling.get_progress_reporting());
    });

    // Setup views
    TaskQueue task_queue;
    ViewStack view_stack;

    std::optional<std::filesystem::path> requested_book_path = (
        argc == 2 ? std::optional<std::filesystem::path>(argv[1]) : std::nullopt
    );
    initialize_views(
        view_stack,
        state_store,
        sys_styling,
        token_view_styling,
        task_queue,
        requested_book_path
    );
    quit = view_stack.is_done();

    std::shared_ptr<SettingsView> settings_view = std::make_shared<SettingsView>(
        sys_styling,
        token_view_styling,
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
            SW_BTN_R1,
            SW_BTN_L2,
            SW_BTN_R2
        }
    );
    SystemKeyChordTracker chord_tracker;

    auto key_held_callback = [&view_stack](SDLKey key, uint32_t held_ms) {
        // Apply rotation to key
        SDLKey rotated_key = get_rotated_keymap(key, current_rotation);
        view_stack.on_keyheld(rotated_key, held_ms);
    };

    // Timing
    Timer idle_timer;
    FPSLimiter limit_fps(TARGET_FPS);
    const uint32_t avg_loop_time = 1000 / TARGET_FPS;

    // Initial render
    view_stack.render(screen, true);
    apply_screen_rotation(screen, video);
    SDL_Flip(video);

    while (!quit)
    {
        bool ran_user_code = task_queue.drain();

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
                        SDLKey key = get_rotated_keymap(chord_tracker.on_keypress(event.key.keysym.sym), current_rotation);
                        

                        if (key == SW_BTN_POWER)
                        {
                            state_store.flush();
                        }
                        else
                        {
                            view_stack.on_keypress(key);

                            if (key == SW_BTN_X)
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
                            }
                            
                            // Toggle rotation
                            if (key == SW_BTN_Y)
                            {
                                std::cout << "applying rotation\n";
                                // Cycle through rotation modes: None -> 90 -> 180 -> 270 -> None
                                switch (current_rotation) {
                                    case ROTATION_NONE:
                                        // std::cout << "applying rotation 90\n";
                                        current_rotation = ROTATION_90;
                                        break;
                                    case ROTATION_90:
                                        // std::cout << "applying rotation 180\n";
                                        current_rotation = ROTATION_180;
                                        break;
                                    case ROTATION_180:
                                        // std::cout << "applying rotation 270\n";
                                        current_rotation = ROTATION_270;
                                        break;
                                    case ROTATION_270:
                                        // std::cout << "applying rotation none\n";
                                        current_rotation = ROTATION_NONE;
                                        break;
                                }

                                auto tmp = SCREEN_WIDTH;
                                SCREEN_WIDTH = SCREEN_HEIGHT;
                                SCREEN_HEIGHT = tmp;
                                
                                // Force re-render with new rotation
                                view_stack.render(screen, true);
                                apply_screen_rotation(screen, video);
                                SDL_Flip(video);
                            }

                            ran_user_code = true;
                        }
                    }
                    break;
                case SDL_KEYUP:
                    {
                        SDLKey key = event.key.keysym.sym;
                        chord_tracker.on_keyrelease(key);
                    }
                    break;
                default:
                    break;
            }
        }

        quit = quit || chord_tracker.exit_requested();

        held_key_tracker.accumulate(avg_loop_time); // Pretend perfect loop timing for event firing consistency
        ran_user_code = held_key_tracker.for_longest_held(key_held_callback) || ran_user_code;

        if (ran_user_code)
        {
            bool force_render = view_stack.pop_completed_views();

            if (view_stack.is_done())
            {
                quit = true;
            }

            if (view_stack.render(screen, force_render))
            {
                apply_screen_rotation(screen, video);
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
