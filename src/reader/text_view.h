#ifndef TEXT_VIEW_H_
#define TEXT_VIEW_H_

#include "./view.h"

#include <memory>
#include <vector>
#include <string>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_video.h>

struct TextViewState;

class TextView: public View
{
    std::unique_ptr<TextViewState> state;
    TTF_Font *font;
    int line_height;
    int line_padding;
    bool _is_done = false;

public:
    TextView(std::vector<std::string> lines, TTF_Font *font, int line_padding);
    virtual ~TextView();

    bool render(SDL_Surface *dest_surface);
    bool on_keypress(SDLKey key);
    bool is_done();
};

#endif
