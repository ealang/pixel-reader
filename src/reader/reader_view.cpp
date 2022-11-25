#include "./reader_view.h"

#include "./display_lines.h"
#include "./selection_menu.h"
#include "./text_view.h"
#include "./view_stack.h"

#include "epub/epub_reader.h"
#include "sys/keymap.h"
#include "sys/screen.h"

#include <iostream>

struct ReaderViewState
{

    bool is_zombie = false;
    bool is_done = false;

    std::function<void()> on_quit;
    EPubReader reader;
    TTF_Font *font;
    uint32_t chapter_index = 0;

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

    // TODO
    std::vector<Line> display_lines;
    get_display_lines(
        tokens,
        line_fits_on_screen,
        display_lines
    );

    std::vector<std::string> lines;
    for (auto &line : display_lines)
    {
        lines.emplace_back(line.text);
    }

    return std::make_shared<TextView>(
        lines,
        state.font
    );
}

void open_chapter_select(ReaderView &reader_view, ReaderViewState &state)
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
    chapter_select->set_cursor_pos(state.chapter_index);

    state.view_stack.push(chapter_select);
}

} // namespace

ReaderView::ReaderView(std::filesystem::path path, TTF_Font *font, ViewStack &view_stack)
    : state(std::make_unique<ReaderViewState>(path, font, view_stack))
{
    if (!state->reader.open())
    {
        std::cerr << "Error opening " << path << std::endl;
        state->text_view = make_error_view(*state);
        state->is_zombie = true;
    }
    else
    {
        state->text_view = make_chapter_view(*state, 0);
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
            if (state->on_quit)
            {
                state->on_quit();
            }
            break;
        case SW_BTN_SELECT:
            if (!state->is_zombie)
            {
                open_chapter_select(*this, *state);
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

void ReaderView::set_on_quit(std::function<void()> callback)
{
    state->on_quit = callback;
}

void ReaderView::open_chapter(uint32_t chapter_index)
{
    state->text_view = make_chapter_view(*state, chapter_index);
    state->chapter_index = chapter_index;
}
