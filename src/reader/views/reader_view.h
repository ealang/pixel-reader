#ifndef READER_VIEW_H_
#define READER_VIEW_H_

#include "doc_api/doc_addr.h"
#include "reader/view.h"

#include <filesystem>
#include <functional>
#include <string>

struct ReaderViewState;
struct SystemStyling;
struct TokenViewStyling;
struct ViewStack;

class ReaderView: public View
{
    std::unique_ptr<ReaderViewState> state;

    void update_token_view_title(DocAddr address);
    void render_error_state(SDL_Surface *dest_surface) const;

public:
    ReaderView(
        std::filesystem::path path,
        DocAddr seek_address,
        SystemStyling &sys_styling,
        TokenViewStyling &token_view_styling,
        ViewStack &view_stack
    );
    virtual ~ReaderView();

    bool render(SDL_Surface *dest_surface, bool force_render) override;
    bool is_done() override;

    void on_keypress(SDLKey key) override;
    void on_keyheld(SDLKey key, uint32_t hold_time_ms) override;
    void on_pop() override;

    void set_on_quit_requested(std::function<void()> callback);
    void set_on_change_address(std::function<void(DocAddr)> callback);

    void seek_to_toc_index(uint32_t toc_index);
    void seek_to_address(DocAddr address);
};

#endif
