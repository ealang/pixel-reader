#ifndef TEXT_VIEW_H_
#define TEXT_VIEW_H_

#include <memory>
#include <vector>
#include <string>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_video.h>

struct TextViewState;

class TextView
{
    std::unique_ptr<TextViewState> state;
    TTF_Font *font;
    int line_height;
    int line_padding;

public:
    TextView(std::vector<std::string> lines, TTF_Font *font, int line_padding);
    virtual ~TextView();

    bool render(SDL_Surface *dest_surface) const;
    bool on_keypress(SDLKey key);
};

#endif
