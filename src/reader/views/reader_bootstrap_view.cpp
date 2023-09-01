#include "./reader_bootstrap_view.h"

#include "filetypes/open_doc.h"
#include "reader/config.h"
#include "reader/ss_doc_reader_cache.h"
#include "reader/state_store.h"
#include "reader/view_stack.h"
#include "./popup_view.h"
#include "./reader_view.h"

#include <iostream>

struct ReaderBootstrapViewState
{
    std::filesystem::path book_path;
    SystemStyling &sys_styling;
    TokenViewStyling &token_view_styling;
    ViewStack &view_stack;
    StateStore &state_store;

    std::unique_ptr<PopupView> loading_modal;
    bool is_done = false;

    ReaderBootstrapViewState(
        std::filesystem::path book_path,
        SystemStyling &sys_styling,
        TokenViewStyling &token_view_styling,
        ViewStack &view_stack,
        StateStore &state_store
    ) :
        book_path(book_path),
        sys_styling(sys_styling),
        token_view_styling(token_view_styling),
        view_stack(view_stack),
        state_store(state_store),
        loading_modal(std::make_unique<PopupView>(
            "Loading...",
            SYSTEM_FONT,
            sys_styling
        ))
    {
    }
};

void ReaderBootstrapView::load_reader()
{
    state->is_done = true;

    auto &book_path = state->book_path;
    auto &sys_styling = state->sys_styling;
    auto &token_view_styling = state->token_view_styling;
    auto &view_stack = state->view_stack;
    auto &state_store = state->state_store;

    std::shared_ptr<DocReader> reader = create_doc_reader(book_path);
    SSDocReaderCache cache(state_store);
    if (!reader || !reader->open(cache))
    {
        std::cerr << "Failed to open " << book_path << std::endl;
        view_stack.push(std::make_shared<PopupView>("Error opening", SYSTEM_FONT, sys_styling));

        return;
    }

    state_store.set_current_book_path(book_path);

    auto book_id = reader->get_id();
    auto reader_view = std::make_shared<ReaderView>(
        book_path,
        reader,
        state_store.get_book_address(book_id).value_or(make_address()),
        sys_styling,
        token_view_styling,
        view_stack
    );

    reader_view->set_on_change_address([&state_store, book_id](DocAddr addr) {
        state_store.set_book_address(book_id, addr);
    });
    reader_view->set_on_quit_requested([&state_store]() {
        state_store.remove_current_book_path();
    });

    view_stack.push(reader_view);
}

ReaderBootstrapView::ReaderBootstrapView(
    std::filesystem::path book_path,
    SystemStyling &sys_styling,
    TokenViewStyling &token_view_styling,
    ViewStack &view_stack,
    StateStore &state_store,
    std::function<void(std::function<void()>)> async
) : state(std::make_unique<ReaderBootstrapViewState>(book_path, sys_styling, token_view_styling, view_stack, state_store))
{
    // Perform asynchronously so that rendering can continue
    async([this](){ load_reader(); });
}

ReaderBootstrapView::~ReaderBootstrapView()
{
}

bool ReaderBootstrapView::render(SDL_Surface *dest_surface, bool force_render)
{
    if (!state->is_done)
    {
        return state->loading_modal->render(dest_surface, force_render);
    }

    return false;
}

bool ReaderBootstrapView::is_done()
{
    return state->is_done;
}

bool ReaderBootstrapView::is_modal()
{
    return true;
}

void ReaderBootstrapView::on_keypress(SDLKey)
{
}
