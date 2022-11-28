#ifndef TEXT_VIEW_H_
#define TEXT_VIEW_H_

#include "reader/view.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

struct TextViewState;

class TextView: public View
{
    std::unique_ptr<TextViewState> state;
    TTF_Font *font;
    int line_height;
    int line_padding = 2;

    void scroll_up(int num_lines);
    void scroll_down(int num_lines);

public:
    TextView(std::vector<std::string> lines, TTF_Font *font);
    virtual ~TextView();

    bool render(SDL_Surface *dest_surface) override;
    bool on_keypress(SDLKey key) override;
    bool is_done() override;
    void on_lose_focus() override;

    uint32_t get_line_number() const;
    void set_line_number(uint32_t line_number);

    // user tried to scroll up when already at the top
    void set_on_resist_up(std::function<void()> callback);

    // user tried to scroll down when already at the bottom
    void set_on_resist_down(std::function<void()> callback);
};

#endif
