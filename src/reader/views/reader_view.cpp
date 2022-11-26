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

    std::function<void()> on_quit;
    std::function<void(const DocAddr &)> on_change_address;

    uint32_t current_chapter_index = -1;
    std::vector<uint32_t> line_addresses;

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

std::shared_ptr<TextView> make_error_view(ReaderViewState &state)
{
    std::vector<std::string> lines = {"Error loading"};
    return std::make_shared<TextView>(
        lines,
        state.font
    );
}

std::shared_ptr<TextView> make_chapter_view(ReaderViewState &state, uint32_t chapter_index)
{
    const auto &tok = state.reader.get_tok();
    if (chapter_index >= tok.size())
    {
        return make_error_view(state);
    }

    auto line_fits_on_screen = [font=state.font](const char *s, uint32_t len) {
        int w = SCREEN_WIDTH, h;

        char *mut_s = (char*)s;
        char replaced = mut_s[len];
        mut_s[len] = 0;

        TTF_SizeUTF8(font, mut_s, &w, &h);
        mut_s[len] = replaced;

        return w <= SCREEN_WIDTH;
    };

    std::cerr << "Loading chapter " << tok[chapter_index].name << std::endl;
    auto chapter = tok[chapter_index];
    auto tokens = state.reader.get_tokenized_document(chapter.doc_id);
    std::cerr << "Got " << tokens.size() << " tokens" << std::endl;

    std::vector<std::string> text_lines;
    state.line_addresses.clear();

    get_display_lines(
        tokens,
        line_fits_on_screen,
        [&text_lines, &addresses=state.line_addresses](const std::string &text, const DocAddr &start_addr) {
            text_lines.push_back(text);
            addresses.push_back(get_text_number(start_addr));
        }
    );

    return std::make_shared<TextView>(
        text_lines,
        state.font
    );
}

void open_chapter_menu(ReaderView &reader_view, ReaderViewState &state)
{
    std::vector<std::string> chapters;
    for (const auto &tok: state.reader.get_tok())
    {
        chapters.emplace_back(tok.name);
    }

    auto chapter_select = std::make_shared<SelectionMenu>(chapters, state.font);
    chapter_select->set_on_selection([&reader_view](uint32_t chapter_index) {
        return reader_view.open_chapter(chapter_index);
    });
    chapter_select->set_close_on_select();
    chapter_select->set_cursor_pos(state.current_chapter_index);

    state.view_stack.push(chapter_select);
}

DocAddr get_current_address(ReaderViewState &state)
{
    uint32_t text_address = 0;
    uint32_t line_index = state.text_view->get_line_number();
    if (line_index < state.line_addresses.size())
    {
        text_address = state.line_addresses[line_index];
    }

    return make_address(
        state.current_chapter_index,
        text_address
    );
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
        state->text_view = make_error_view(*state);
        state->is_zombie = true;
    }
    else
    {
        open_address(seek_address);
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
                open_chapter_menu(*this, *state);
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
    if (state->on_change_address)
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

void ReaderView::open_chapter(uint32_t chapter_index)
{
    if (chapter_index != state->current_chapter_index)
    {
        state->text_view = make_chapter_view(*state, chapter_index);
        state->current_chapter_index = chapter_index;
    }
}

void ReaderView::open_address(DocAddr address)
{
    // Find chapter
    open_chapter(get_chapter_number(address));

    // Find line
    uint32_t text_address = get_text_number(address);

    uint32_t best_line_number = 0;
    uint32_t line_number = 0;
    for (const auto &line_addr: state->line_addresses)
    {
        if (line_addr <= text_address)
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
