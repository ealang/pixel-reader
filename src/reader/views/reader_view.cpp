#include "./reader_view.h"

#include "./selection_menu.h"
#include "./token_view/token_view.h"
#include "./token_view/token_view_styling.h"

#include "reader/system_styling.h"
#include "reader/view_stack.h"

#include "epub/epub_reader.h"
#include "sys/keymap.h"
#include "sys/screen.h"
#include "util/sdl_font_cache.h"

#include <iostream>

struct ReaderViewState
{
    bool is_error_state = false;
    bool is_done = false;

    std::function<void()> on_quit;
    std::function<void(DocAddr)> on_change_address;

    EPubReader reader;
    SystemStyling &sys_styling;
    TokenViewStyling &token_view_styling;

    ViewStack &view_stack;

    std::unique_ptr<TokenView> token_view;
    
    ReaderViewState(std::filesystem::path path, SystemStyling &sys_styling, TokenViewStyling &token_view_styling, ViewStack &view_stack)
        : reader(path),
          sys_styling(sys_styling),
          token_view_styling(token_view_styling),
          view_stack(view_stack)
    {
    }

    ~ReaderViewState()
    {
    }
};

namespace
{

DocAddr get_current_address(const ReaderViewState &state)
{
    if (state.token_view)
    {
        return state.token_view->get_address();
    }

    return make_address();
}

void open_toc_menu(ReaderView &reader_view, ReaderViewState &state)
{
    const auto &toc = state.reader.get_table_of_contents();
    if (toc.empty())
    {
        return;
    }

    // setup toc entries & callbacks
    std::vector<std::string> menu_names;
    for (const auto &toc_item: toc)
    {
        std::string indent(toc_item.indent_level * 2, ' ');
        menu_names.push_back(indent + toc_item.display_name);
    }

    auto current_toc_index = state.reader.get_toc_position(get_current_address(state)).toc_index;
    auto toc_select_menu = std::make_shared<SelectionMenu>(
        menu_names,
        state.sys_styling,
        state.token_view_styling.get_font()
    );
    toc_select_menu->set_on_selection([&reader_view, last_toc_index=current_toc_index](uint32_t toc_index) {
        if (toc_index != last_toc_index)
        {
            reader_view.seek_to_toc_index(toc_index);
        }
    });
    toc_select_menu->set_close_on_select();

    // select current toc item
    if (current_toc_index < toc.size())
    {
        toc_select_menu->set_cursor_pos(current_toc_index);
    }

    toc_select_menu->set_default_on_keypress([](SDLKey key, SelectionMenu &toc) {
        if (key == SW_BTN_SELECT)
        {
            toc.close();
        }
    });

    state.view_stack.push(toc_select_menu);
}

} // namespace

ReaderView::ReaderView(
    std::filesystem::path path,
    DocAddr seek_address,
    SystemStyling &sys_styling,
    TokenViewStyling &token_view_styling,
    ViewStack &view_stack
) : state(std::make_unique<ReaderViewState>(path, sys_styling, token_view_styling, view_stack))
{
    if (!state->reader.open())
    {
        std::cerr << "Error opening " << path << std::endl;
        state->is_error_state = true;
    }
    else
    {
        state->token_view = std::make_unique<TokenView>(
            state->reader,
            seek_address,
            sys_styling,
            token_view_styling
        );

        update_token_view_title(seek_address);

        // Update title info on scroll
        state->token_view->set_on_scroll([this](DocAddr address) {
            update_token_view_title(address);
        });
    }
}

ReaderView::~ReaderView()
{
}

void ReaderView::update_token_view_title(DocAddr address)
{
    const auto &toc = state->reader.get_table_of_contents();
    auto toc_position = state->reader.get_toc_position(address);
    if (toc_position.toc_index < toc.size())
    {
        state->token_view->set_title(toc[toc_position.toc_index].display_name);
        state->token_view->set_title_progress(toc_position.progress_percent);
    }
}

void ReaderView::render_error_state(SDL_Surface *dest_surface) const
{
    const auto &theme = state->sys_styling.get_loaded_color_theme();
    TTF_Font *font = cached_load_font(state->token_view_styling.get_font(), state->sys_styling.get_font_size());

    // clear screen
    SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    const auto &bgcolor = theme.background;
    SDL_FillRect(
        dest_surface,
        &rect,
        SDL_MapRGB(dest_surface->format, bgcolor.r, bgcolor.g, bgcolor.b)
    );

    SDL_Surface *surface = TTF_RenderUTF8_Shaded(font, "Error loading", theme.main_text, theme.background);
    SDL_BlitSurface(surface, nullptr, dest_surface, nullptr);
    SDL_FreeSurface(surface);
}

bool ReaderView::render(SDL_Surface *dest_surface, bool force_render)
{
    if (state->is_error_state)
    {
        render_error_state(dest_surface);
        return true;
    }

    return state->token_view->render(dest_surface, force_render);
}

bool ReaderView::is_done()
{
    return state->is_done;
}

void ReaderView::on_keypress(SDLKey key)
{
    if (key == SW_BTN_B)
    {
        state->is_done = true;
        if (state->on_quit)
        {
            state->on_quit();
        }
        return;
    }

    if (state->is_error_state)
    {
        return;
    }

    switch (key) {
        case SW_BTN_A:
            {
                bool show = !state->token_view_styling.get_show_title_bar();
                state->token_view_styling.set_show_title_bar(show);
                state->token_view->set_show_title_bar(show);
            }
            break;
        case SW_BTN_SELECT:
            open_toc_menu(*this, *state);
            break;
        default:
            state->token_view->on_keypress(key);
            break;
    }
}

void ReaderView::on_keyheld(SDLKey key, uint32_t hold_time_ms)
{
    if (state->is_error_state)
    {
        return;
    }
    state->token_view->on_keyheld(key, hold_time_ms);
}

void ReaderView::on_pop()
{
    if (!state->is_error_state && state->on_change_address)
    {
        state->on_change_address(
            get_current_address(*state)
        );
    }
}

void ReaderView::set_on_quit_requested(std::function<void()> callback)
{
    state->on_quit = callback;
}

void ReaderView::set_on_change_address(std::function<void(DocAddr)> callback)
{
    state->on_change_address = callback;
}

void ReaderView::seek_to_toc_index(uint32_t toc_index)
{
    auto address = state->reader.get_toc_item_address(toc_index);
    seek_to_address(address);
}

void ReaderView::seek_to_address(DocAddr address)
{
    if (state->token_view)
    {
        state->token_view->seek_to_address(address);
        update_token_view_title(address);
    }
}
