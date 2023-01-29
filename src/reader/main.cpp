#include "./config.h"
#include "./font_catalog.h"
#include "./range_db.h"
#include "./settings_store.h"
#include "./shoulder_keymap.h"
#include "./state_store.h"
#include "./system_styling.h"
#include "./color_theme_def.h"
#include "./view_stack.h"
#include "./views/file_selector.h"
#include "./views/popup_view.h"
#include "./views/reader_view.h"
#include "./views/settings_view.h"
#include "./views/token_view/token_view_styling.h"
#include "doc_api/token_addressing.h"
#include "filetypes/open_doc.h"
#include "sys/keymap.h"
#include "sys/screen.h"
#include "util/cli_render_lines.h"
#include "util/fps_limiter.h"
#include "util/held_key_tracker.h"
#include "util/key_value_file.h"
#include "util/sdl_font_cache.h"
#include "util/timer.h"

#include <libxml/parser.h>
#include <SDL/SDL.h>

#include <csignal>
#include <fstream>
#include <iostream>
#include <set>

namespace
{

void export_notes(std::shared_ptr<DocReader> reader, std::filesystem::path book_path, const Ranges &highlights)
{
    const int col_width = 40;

    std::filesystem::path notes_path = book_path;
    notes_path.replace_extension(".notes.txt");
    std::ofstream notes_file(notes_path.string());

    bool needs_separator = false;
    uint32_t last_toc_index = -1;
    auto write_line = [&last_toc_index, &notes_file, &reader, &needs_separator](const Line &line) {
        const auto &toc = reader->get_table_of_contents();
        auto toc_pos = reader->get_toc_position(line.address);
        if (toc_pos.toc_index < toc.size() && toc_pos.toc_index != last_toc_index)
        {
            notes_file << std::string(col_width, '=') << std::endl;
            notes_file << "Chapter: " << toc[toc_pos.toc_index].display_name << std::endl << std::endl;
            last_toc_index = toc_pos.toc_index;
        }
        else if (needs_separator)
        {
            notes_file << std::string(col_width, '-') << std::endl;
        }

        notes_file << line.text << std::endl;
        needs_separator = false;
    };

    for (const auto &[hl_start, hl_end] : highlights)
    {
        auto it = reader->get_iter(hl_start);

        const DocToken *token = nullptr;
        while ((token = it->read(1)) != nullptr)
        {
            std::vector<const DocToken *> tokens = { token };
            for (const auto &line : cli_render_tokens(tokens, col_width))
            {
                DocAddr line_start = line.address;
                DocAddr line_end = line_start + get_address_width(line.text);
                if (hl_start >= line_end)
                {
                    continue;
                }
                if (hl_end <= line_start)
                {
                    break;
                }

                write_line(line);
            }
        }

        needs_separator = true;
    }
}

class NotesExportQueue
{
    std::set<std::filesystem::path> paths;

    void export_notes(const StateStore &state_store, const std::filesystem::path &book_path)
    {
        std::cerr << "Export notes for " << book_path << std::endl;
        std::shared_ptr<DocReader> reader = create_doc_reader(book_path);
        if (!reader || !reader->open())
        {
            std::cerr << "Failed to open " << book_path << std::endl;
            return;
        }

        auto book_id = reader->get_id();

        ::export_notes(
            reader,
            book_path,
            state_store.get_book_highlights(book_id)
        );
    }
public:
    void add_job(std::filesystem::path book_path)
    {
        paths.insert(book_path);
    }

    void export_notes(const StateStore &state_store)
    {
        for (const auto &path : paths)
        {
            export_notes(state_store, path);
        }
        paths.clear();
    }
};

class StoreBackedReaderCache: public ReaderDataCache
{
    StateStore &store;
    ViewStack &view_stack;
    SystemStyling &sys_styling;
    std::function<void()> request_render;
    std::shared_ptr<PopupView> message_view = nullptr;

public:
    StoreBackedReaderCache(
        StateStore &store,
        ViewStack &view_stack,
        SystemStyling &sys_styling,
        std::function<void()> request_render
    )
        : store(store), view_stack(view_stack), sys_styling(sys_styling), request_render(request_render)
    {
    }

    std::unordered_map<std::string, std::string> load_book_cache(const std::string &book_id) override
    {
        return store.get_book_cache(book_id);
    }

    void set_book_cache(const std::string &book_id, std::unordered_map<std::string, std::string> &data) override
    {
        store.set_book_cache(book_id, data);
    }

    void report_load_status(const std::string &message) override
    {
        if (message_view)
        {
            message_view->close();
        }

        message_view = std::make_shared<PopupView>(message, true, SYSTEM_FONT, sys_styling);
        view_stack.push(message_view);
        request_render();
    }

    void report_load_complete()
    {
        if (message_view)
        {
            message_view->close();
            message_view = nullptr;
        }
    }
};

void initialize_views(
    ViewStack &view_stack,
    StateStore &state_store,
    SystemStyling &sys_styling,
    TokenViewStyling &token_view_styling,
    std::shared_ptr<StoreBackedReaderCache> reader_cache,
    NotesExportQueue &notes_export_queue
)
{
    auto browse_path = state_store.get_current_browse_path().value_or(DEFAULT_BROWSE_PATH);
    std::shared_ptr<FileSelector> fs = std::make_shared<FileSelector>(
        browse_path,
        sys_styling
    );

    auto load_book = [&view_stack, &state_store, &sys_styling, &token_view_styling, reader_cache, &notes_export_queue](std::filesystem::path path) {
        if (!std::filesystem::exists(path) || !file_type_is_supported(path))
        {
            return;
        }

        std::shared_ptr<DocReader> reader = create_doc_reader(path);
        if (!reader || !reader->open(reader_cache))
        {
            std::cerr << "Failed to open " << path << std::endl;
            reader_cache->report_load_complete();
            view_stack.push(std::make_shared<PopupView>("Error opening", true, SYSTEM_FONT, sys_styling));
            return;
        }
        reader_cache->report_load_complete();

        state_store.set_current_book_path(path);

        auto book_id = reader->get_id();

        RangeDB highlights(state_store.get_book_highlights(book_id));
        auto reader_view = std::make_shared<ReaderView>(
            path,
            reader,
            state_store.get_book_address(book_id).value_or(make_address()),
            highlights,
            sys_styling,
            token_view_styling,
            view_stack
        );

        reader_view->set_on_change_address([&state_store, book_id](DocAddr addr) {
            state_store.set_book_address(book_id, addr);
        });

        reader_view->set_on_highlight([&state_store, book_id, &view_stack, &sys_styling, &notes_export_queue, path](const RangeDB &highlights) {
            state_store.set_book_highlights(book_id, highlights.get_ranges());
            notes_export_queue.add_job(path);
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

uint32_t bound(uint32_t val, uint32_t min, uint32_t max)
{
    return std::max(std::min(val, max), min);
}

const char *CONFIG_KEY_STORE_PATH = "store_path";

std::unordered_map<std::string, std::string> load_config_with_defaults()
{
    auto config = load_key_value(CONFIG_FILE_PATH);
    config.try_emplace(CONFIG_KEY_STORE_PATH, FALLBACK_STORE_PATH);
    return config;
}

void render_views(ViewStack &view_stack, SDL_Surface *screen, SDL_Surface *video, bool force_render)
{
    if (view_stack.render(screen, force_render))
    {
        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
    }
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
    TokenViewStyling token_view_styling(settings_get_show_title_bar(state_store).value_or(DEFAULT_SHOW_PROGRESS));
    token_view_styling.subscribe_to_changes([&token_view_styling, &state_store]() {
        // Persist changes
        settings_set_show_title_bar(state_store, token_view_styling.get_show_title_bar());
    });

    // Setup views
    NotesExportQueue notes_export_queue;
    ViewStack view_stack;
    auto reader_cache = std::make_shared<StoreBackedReaderCache>(
        state_store,
        view_stack,
        sys_styling,
        [&]() {
            render_views(view_stack, screen, video, false);
        }
    );
    initialize_views(
        view_stack,
        state_store,
        sys_styling,
        token_view_styling,
        reader_cache,
        notes_export_queue
    );

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
            SW_BTN_R1,
            SW_BTN_L2,
            SW_BTN_R2
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
    render_views(view_stack, screen, video, true);
    SDL_Flip(video);

    while (!quit)
    {
        bool ran_user_code = false;

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
                            notes_export_queue.export_notes(state_store);
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

                            ran_user_code = true;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        held_key_tracker.accumulate(avg_loop_time); // Pretend perfect loop timing for event firing consistency
        ran_user_code = held_key_tracker.for_longest_held(key_held_callback) || ran_user_code;

        if (ran_user_code)
        {
            bool force_render = view_stack.pop_completed_views();

            if (view_stack.is_done())
            {
                quit = true;
            }

            render_views(view_stack, screen, video, force_render);
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

    notes_export_queue.export_notes(state_store);
    view_stack.shutdown();
    state_store.flush();

    SDL_FreeSurface(screen);
    SDL_Quit();
    xmlCleanupParser();
    
    return 0;
}
