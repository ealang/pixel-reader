#ifndef READER_VIEW_H_
#define READER_VIEW_H_

#include "./view.h"

#include <filesystem>
#include <functional>
#include <string>

class ViewStack;
struct ReaderViewState;

class ReaderView: public View
{
    std::unique_ptr<ReaderViewState> state;

public:
    ReaderView(std::filesystem::path path, TTF_Font *font, ViewStack &view_stack);
    virtual ~ReaderView();

    bool render(SDL_Surface *dest_surface) override;
    bool on_keypress(SDLKey key) override;
    bool is_done() override;
    void on_lose_focus() override;

    void open_chapter(uint32_t chapter_index);

    void set_on_quit(std::function<void()> callback);
};

#endif
