#include "./reader_view.h"

#include "reader/display_lines.h"
#include "reader/view_stack.h"
#include "./selection_menu.h"
#include "./text_view.h"

#include "epub/epub_reader.h"
#include "sys/keymap.h"
#include "sys/screen.h"

#include <iostream>

struct ReaderViewState
{

    bool is_zombie = false;
    bool is_done = false;
    bool has_data = false;

    DocAddr last_loaded_address = 0;
    std::function<void()> on_quit;
    std::function<void(const DocAddr &)> on_change_address;
    std::vector<DocAddr> line_addresses;

    EPubReader reader;
    TTF_Font *font;

    ViewStack &view_stack;
    std::shared_ptr<TextView> text_view;
    
    ReaderViewState(std::filesystem::path path, TTF_Font *font, ViewStack &view_stack)
        : reader(path), font(font), view_stack(view_stack)
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
        state.font
    );
}

std::shared_ptr<TextView> make_text_view(ReaderView &reader_view, ReaderViewState &state, const DocAddr &address)
{
    auto line_fits_on_screen = [font=state.font](const char *s, uint32_t len) {
        int w = SCREEN_WIDTH, h;

        char *mut_s = (char*)s;
        char replaced = mut_s[len];
        mut_s[len] = 0;

        TTF_SizeUTF8(font, mut_s, &w, &h);
        mut_s[len] = replaced;

        return w <= SCREEN_WIDTH;
    };

    auto tokens = state.reader.get_tokenized_document(address);

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
        state.font
    );

    text_view->set_on_resist_up([&reader_view]() {
        reader_view.seek_to_prev_doc();
    });

    text_view->set_on_resist_down([&reader_view]() {
        reader_view.seek_to_next_doc();
    });

    return text_view;
}

DocAddr get_current_address(ReaderViewState &state)
{
    uint32_t line_index = state.text_view->get_line_number();
    if (line_index < state.line_addresses.size())
    {
        return state.line_addresses[line_index];
    }

    return state.last_loaded_address;
}

void open_toc_menu(ReaderView &reader_view, ReaderViewState &state)
{
    if (state.reader.get_table_of_contents().empty())
    {
        return;
    }

    // setup toc entries & callbacks
    std::vector<std::string> menu_names;
    for (const auto &toc_item: state.reader.get_table_of_contents())
    {
        std::string indent(toc_item.indent_level * 2, ' ');
        menu_names.push_back(indent + toc_item.display_name);
    }

    auto toc_select_menu = std::make_shared<SelectionMenu>(menu_names, state.font);
    toc_select_menu->set_on_selection([&reader_view](uint32_t toc_index) {
        return reader_view.seek_to_toc_index(toc_index);
    });
    toc_select_menu->set_close_on_select();

    // select current toc item
    uint32_t current_toc_index = state.reader.get_toc_index(get_current_address(state));
    toc_select_menu->set_cursor_pos(current_toc_index);

    state.view_stack.push(toc_select_menu);
}

} // namespace

ReaderView::ReaderView(
    std::filesystem::path path,
    DocAddr seek_address,
    TTF_Font *font,
    ViewStack &view_stack
) : state(std::make_unique<ReaderViewState>(path, font, view_stack))
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
    }
}

ReaderView::~ReaderView()
{
}

bool ReaderView::render(SDL_Surface *dest_surface)
{
    return state->text_view->render(dest_surface);
}

bool ReaderView::on_keypress(SDLKey key)
{
    switch (key) {
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
            return state->text_view->on_keypress(key);
    }

    return true;
}

bool ReaderView::is_done()
{
    return state->is_done;
}

void ReaderView::on_lose_focus()
{
    state->text_view->on_lose_focus();
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

void ReaderView::seek_to_toc_index(uint32_t new_toc_index)
{
    DocAddr address = get_current_address(*state);
    uint32_t cur_toc_index = state->reader.get_toc_index(address);

    if (!state->has_data || new_toc_index != cur_toc_index)
    {
        const auto &toc = state->reader.get_table_of_contents();
        if (new_toc_index < toc.size())
        {
            const auto &toc_item = toc[new_toc_index];
            seek_to_address(toc_item.address);
        }
    }
}

void ReaderView::seek_to_address(const DocAddr &address)
{
    // Load document
    state->has_data = true;
    state->text_view = make_text_view(*this, *state, address);
    state->last_loaded_address = address;

    // Find line number
    uint32_t best_line_number = 0;
    uint32_t line_number = 0;
    for (const auto &line_addr: state->line_addresses)
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

    state->text_view->set_line_number(best_line_number);
}

void ReaderView::seek_to_prev_doc()
{
    DocAddr address = get_current_address(*state);
    auto next_address = state->reader.get_previous_doc_address(address);
    if (next_address)
    {
        seek_to_address(*next_address);

        if (!state->line_addresses.empty())
        {
            state->text_view->set_line_number(state->line_addresses.size() - 1);
        }
    }
}

void ReaderView::seek_to_next_doc()
{
    DocAddr address = get_current_address(*state);
    auto next_address = state->reader.get_next_doc_address(address);
    if (next_address)
    {
        seek_to_address(*next_address);
    }
}

