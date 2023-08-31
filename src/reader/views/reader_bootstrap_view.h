#ifndef READER_BOOTSTRAP_VIEW_H_
#define READER_BOOTSTRAP_VIEW_H_

#include "doc_api/doc_addr.h"
#include "reader/view.h"

struct ReaderBootstrapViewState;
struct SystemStyling;
struct TokenViewStyling;
struct ViewStack;
struct StateStore;

#include <filesystem>
#include <functional>
#include <memory>

// Temporary view to open a book and display loading/error message
class ReaderBootstrapView: public View
{
    std::unique_ptr<ReaderBootstrapViewState> state;

    void load_reader();

public:
    ReaderBootstrapView(
        std::filesystem::path book_path,
        SystemStyling &sys_styling,
        TokenViewStyling &token_view_styling,
        ViewStack &view_stack,
        StateStore &state_store,
        std::function<void(std::function<void()>)> async
    );
    virtual ~ReaderBootstrapView();

    bool render(SDL_Surface *dest_surface, bool force_render) override;
    bool is_done() override;
    bool is_modal() override;
    void on_keypress(SDLKey) override;
};

#endif
