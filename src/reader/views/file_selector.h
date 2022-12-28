#ifndef FILE_SELECTOR_H_
#define FILE_SELECTOR_H_

#include "reader/view.h"

#include <SDL/SDL_video.h>

#include <functional>
#include <filesystem>
#include <memory>
#include <string>

struct FSState;
struct SystemStyling;

class FileSelector: public View
{
    std::unique_ptr<FSState> state;

public:
    // Expects to receive a path to a file, or directory with trailing separator.
    FileSelector(std::filesystem::path path, SystemStyling &styling);
    virtual ~FileSelector();

    bool render(SDL_Surface *dest_surface, bool force_render);
    bool is_done() override;
    void on_keypress(SDLKey key) override;
    void on_keyheld(SDLKey key, uint32_t held_time_ms) override;

    void set_on_file_selected(std::function<void(const std::filesystem::path &)> on_file_selected);
    void set_on_file_focus(std::function<void(const std::filesystem::path &)> on_file_focus);
};

#endif
