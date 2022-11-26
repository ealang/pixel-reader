#ifndef TEXT_VIEW_H_
#define TEXT_VIEW_H_

#include "reader/view.h"

#include <memory>
#include <vector>
#include <string>

struct TextViewState;

class TextView: public View
{
    std::unique_ptr<TextViewState> state;
    TTF_Font *font;
    int line_height;
    int line_padding = 2;

public:
    TextView(std::vector<std::string> lines, TTF_Font *font);
    virtual ~TextView();

    bool render(SDL_Surface *dest_surface) override;
    bool on_keypress(SDLKey key) override;
    bool is_done() override;
    void on_lose_focus() override;
};

#endif
