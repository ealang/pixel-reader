#include "./reader_view.h"

#include "./selection_menu.h"
#include "./text_view.h"
#include "./text_view_styling.h"

#include "reader/color_theme_def.h"
#include "reader/display_lines.h"
#include "reader/system_styling.h"
#include "reader/view_stack.h"

#include "doc_api/token_addressing.h"
#include "epub/epub_reader.h"
#include "sys/keymap.h"
#include "sys/screen.h"
#include "util/sdl_font_cache.h"

#include <iostream>

struct ReaderViewState
{
    bool is_zombie = false;
    bool is_done = false;

    DocAddr last_loaded_address = 0;
    std::function<void()> on_quit;
    std::function<void(const DocAddr &)> on_change_address;
    std::vector<DocAddr> line_addresses;

    EPubReader reader;
    SystemStyling &sys_styling;
    std::optional<uint32_t> sys_styling_sub_id;

    TextViewStyling &text_view_styling;
    uint32_t last_font_size;

    ViewStack &view_stack;
    std::shared_ptr<TextView> text_view;
    
    ReaderViewState(std::filesystem::path path, SystemStyling &sys_styling, TextViewStyling &text_view_styling, ViewStack &view_stack)
        : reader(path),
          sys_styling(sys_styling),
          text_view_styling(text_view_styling),
          last_font_size(sys_styling.get_font_size()),
          view_stack(view_stack)
    {
    }

    ~ReaderViewState()
    {
    }
};

namespace
{

std::shared_ptr<TextView> make_error_text_view(ReaderViewState &state)
{
    std::vector<std::string> lines = {"Error loading"};
    return std::make_shared<TextView>(
        lines,
        state.sys_styling,
        state.text_view_styling
    );
}

std::shared_ptr<TextView> make_text_view(ReaderView &reader_view, ReaderViewState &state, const DocAddr &address)
{
    TTF_Font *font = cached_load_font(state.text_view_styling.get_font(), state.sys_styling.get_font_size());

    auto line_fits_on_screen = [font](const char *s, uint32_t len) {
        int w = SCREEN_WIDTH, h;

        char *mut_s = (char*)s;
        char replaced = mut_s[len];
        mut_s[len] = 0;

        TTF_SizeUTF8(font, mut_s, &w, &h);
        mut_s[len] = replaced;

        return w <= SCREEN_WIDTH;
    };

    const std::vector<DocToken> &tokens = state.reader.load_chapter(address);

    std::vector<std::string> text_lines;
    state.line_addresses.clear();

    get_display_lines(
        tokens,
        line_fits_on_screen,
        [&text_lines, &addresses=state.line_addresses](const std::string &text, const DocAddr &start_addr) {
            text_lines.push_back(text);
            addresses.push_back(start_addr);
        }
    );

    auto text_view = std::make_shared<TextView>(
        text_lines,
        state.sys_styling,
        state.text_view_styling
    );

    // title
    const auto &toc = state.reader.get_table_of_contents();
    auto toc_position = state.reader.get_toc_position(address);
    if (toc_position.toc_index < toc.size())
    {
        text_view->set_title(toc[toc_position.toc_index].display_name);
        text_view->set_title_progress(toc_position.progress_percent);
    }

    // set line number
    if (state.line_addresses.size())
    {
        uint32_t best_line_number = 0;
        uint32_t line_number = 0;
        for (const auto &line_addr: state.line_addresses)
        {
            if (line_addr <= address)
            {
                best_line_number = line_number;
            }
            else
            {
                break;
            }
            ++line_number;
        }
        text_view->set_line_number(best_line_number);
    }
    text_view->set_show_title_bar(state.text_view_styling.get_show_title_bar());

    // Update title info on scroll
    text_view->set_on_scroll([tv_ptr=text_view.get(), &reader=state.reader, &addresses=state.line_addresses](uint32_t line_pos) {
        if (line_pos < addresses.size())
        {
            auto toc_position = reader.get_toc_position(addresses[line_pos]);
            const auto &toc = reader.get_table_of_contents();
            if (toc_position.toc_index < toc.size())
            {
                tv_ptr->set_title(toc[toc_position.toc_index].display_name);
                tv_ptr->set_title_progress(toc_position.progress_percent);
            }
        }
    });

    // Navigation between sections
    text_view->set_on_resist_up([&reader_view]() {
        reader_view.seek_to_previous_chapter();
    });
    text_view->set_on_resist_down([&reader_view]() {
        reader_view.seek_to_next_chapter();
    });

    return text_view;
}

DocAddr get_current_address(const ReaderViewState &state)
{
    uint32_t line_index = state.text_view->get_line_number();
    if (line_index < state.line_addresses.size())
    {
        return state.line_addresses[line_index];
    }

    // Case where the current document is empty
    return state.last_loaded_address;
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
        state.text_view_styling.get_font()
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
    TextViewStyling &text_view_styling,
    ViewStack &view_stack
) : state(std::make_unique<ReaderViewState>(path, sys_styling, text_view_styling, view_stack))
{
    if (!state->reader.open())
    {
        std::cerr << "Error opening " << path << std::endl;
        state->text_view = make_error_text_view(*state);
        state->is_zombie = true;
    }
    else
    {
        seek_to_address(seek_address);
        state->sys_styling_sub_id = sys_styling.subscribe_to_changes([this]() {
            if (state->last_font_size != state->sys_styling.get_font_size())
            {
                state->last_font_size = state->sys_styling.get_font_size();
                recreate_text_view();
            }
        });
    }
}

ReaderView::~ReaderView()
{
    auto &sub_id = state->sys_styling_sub_id;
    if (sub_id)
    {
        state->sys_styling.unsubscribe_from_changes(*sub_id);
    }
}

bool ReaderView::render(SDL_Surface *dest_surface, bool force_render)
{
    return state->text_view->render(dest_surface, force_render);
}

bool ReaderView::is_done()
{
    return state->is_done;
}

void ReaderView::on_keypress(SDLKey key)
{
    switch (key) {
        case SW_BTN_A:
            {
                bool show = !state->text_view_styling.get_show_title_bar();
                state->text_view_styling.set_show_title_bar(show);
                state->text_view->set_show_title_bar(show);
            }
            break;
        case SW_BTN_B:
            state->is_done = true;
            if (!state->is_zombie && state->on_quit)
            {
                state->on_quit();
            }
            break;
        case SW_BTN_SELECT:
            if (!state->is_zombie)
            {
                open_toc_menu(*this, *state);
            }
            break;
        default:
            state->text_view->on_keypress(key);
    }
}

void ReaderView::on_keyheld(SDLKey key, uint32_t hold_time_ms)
{
    state->text_view->on_keyheld(key, hold_time_ms);
}

void ReaderView::on_pop()
{
    if (!state->is_zombie && state->on_change_address)
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

void ReaderView::set_on_change_address(std::function<void(const DocAddr &)> callback)
{
    state->on_change_address = callback;
}

void ReaderView::recreate_text_view()
{
    DocAddr address = get_current_address(*state);
    seek_to_address(address);
}

void ReaderView::seek_to_toc_index(uint32_t toc_index)
{
    auto address = state->reader.get_toc_item_address(toc_index);
    seek_to_address(address);
}

void ReaderView::seek_to_address(const DocAddr &address)
{
    state->last_loaded_address = address;

    // TODO: could avoid recreating if seeking to somewhere else in cur doc
    state->text_view = make_text_view(*this, *state, address);
    if (!state->text_view)
    {
        state->text_view = make_error_text_view(*state);
        state->is_zombie = true;
        return;
    }
}

void ReaderView::seek_to_previous_chapter()
{
    auto prev_addr_opt = state->reader.get_prev_chapter_address(get_current_address(*state));
    if (prev_addr_opt)
    {
        const auto &tokens = state->reader.load_chapter(*prev_addr_opt);

        // Go to last line
        DocAddr new_addr = *prev_addr_opt;
        if (tokens.size())
        {
            const auto &last_token = tokens.back();
            new_addr = last_token.address + get_address_width(last_token) - 1;
        }

        seek_to_address(new_addr);
    }
}

void ReaderView::seek_to_next_chapter()
{
    auto next_addr = state->reader.get_next_chapter_address(get_current_address(*state));
    if (next_addr)
    {
        seek_to_address(*next_addr);
    }
}

