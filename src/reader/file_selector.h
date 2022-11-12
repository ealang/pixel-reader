#ifndef FILE_SELECTOR_H_
#define FILE_SELECTOR_H_

#include <memory>
#include <string>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_video.h>

struct FSState;

class FileSelector
{
    std::unique_ptr<FSState> state;

public:
    FileSelector(const std::string &path, TTF_Font *font, int line_padding);
    virtual ~FileSelector();

    bool file_is_selected() const;
    std::string get_selected_file() const;

    bool render(SDL_Surface *dest_surface) const;
    bool on_keypress(SDLKey key);
};

#endif
