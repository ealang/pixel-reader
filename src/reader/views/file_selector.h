#ifndef FILE_SELECTOR_H_
#define FILE_SELECTOR_H_

#include "reader/view.h"
#include "util/task_queue.h"

#include <SDL/SDL_video.h>

#include <functional>
#include <filesystem>
#include <memory>
#include <string>

struct FSState;
class StateStore;
struct SystemStyling;

class FileSelector: public View
{
    std::shared_ptr<FSState> state;

public:
    // Expects to receive a path to a file, or directory with trailing separator.
    FileSelector(
        std::filesystem::path path,
        StateStore &state_store,
        SystemStyling &styling,
        std::function<void(task_func)> async
    );
    virtual ~FileSelector();

    bool render(SDL_Surface *dest_surface, bool force_render) override;
    bool is_done() override;
    void on_keypress(SDLKey key) override;
    void on_keyheld(SDLKey key, uint32_t held_time_ms) override;
    void on_focus() override;

    void set_on_file_selected(std::function<void(const std::filesystem::path &)> on_file_selected);
    void set_on_file_focus(std::function<void(const std::filesystem::path &)> on_file_focus);
    void set_on_view_focus(std::function<void()> on_view_focus);
};

#endif
