#ifndef FILE_SELECTOR_H_
#define FILE_SELECTOR_H_

#include "reader/view.h"

#include <functional>
#include <filesystem>
#include <memory>
#include <string>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_video.h>

struct FSState;

class FileSelector: public View
{
    std::unique_ptr<FSState> state;

public:
    // Expects to receive a path to a file, or directory with trailing separator.
    FileSelector(std::filesystem::path path, TTF_Font *font);
    virtual ~FileSelector();

    bool render(SDL_Surface *dest_surface);
    bool on_keypress(SDLKey key) override;
    bool is_done() override;

    void set_on_file_selected(std::function<void(const std::filesystem::path &)> on_file_selected);
    void set_on_file_focus(std::function<void(const std::filesystem::path &)> on_file_focus);
};

#endif
