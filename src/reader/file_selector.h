#ifndef FILE_SELECTOR_H_
#define FILE_SELECTOR_H_

#include "./view.h"

#include <functional>
#include <memory>
#include <string>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_video.h>

struct FSState;

class FileSelector: public View
{
    std::unique_ptr<FSState> state;

public:
    FileSelector(std::string path, TTF_Font *font);
    virtual ~FileSelector();

    bool render(SDL_Surface *dest_surface);
    bool on_keypress(SDLKey key) override;
    bool is_done() override;

    void set_on_file_selected(std::function<void(const std::string &)> on_file_selected);
};

#endif
