#ifndef READER_VIEW_H_
#define READER_VIEW_H_

#include "doc_api/doc_addr.h"
#include "reader/view.h"

#include <filesystem>
#include <functional>
#include <string>

class ViewStack;
struct ReaderViewState;

class ReaderView: public View
{
    std::unique_ptr<ReaderViewState> state;

public:
    ReaderView(
        std::filesystem::path path,
        DocAddr seek_address,
        TTF_Font *font,
        ViewStack &view_stack
    );
    virtual ~ReaderView();

    bool render(SDL_Surface *dest_surface) override;
    bool on_keypress(SDLKey key) override;
    bool is_done() override;
    void on_lose_focus() override;
    void on_pop() override;

    void set_on_quit_requested(std::function<void()> callback);
    void set_on_change_address(std::function<void(const DocAddr &)> callback);

    void seek_to_toc_index(uint32_t toc_index);
    void seek_to_address(DocAddr address);
};

#endif
