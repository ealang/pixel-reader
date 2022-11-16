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
    FileSelector(const std::string &path, TTF_Font *font, int line_padding);
    virtual ~FileSelector();

    bool render(SDL_Surface *dest_surface);
    bool on_keypress(SDLKey key);
    bool is_done();

    void set_on_file_selected(std::function<void(const std::string &)> on_file_selected);
};

#endif
