#ifndef TEXT_VIEW_H_
#define TEXT_VIEW_H_

#include "reader/view.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

struct SystemStyling;
struct TextViewState;
struct TextViewStyling;

class TextView: public View
{
    std::unique_ptr<TextViewState> state;

    void scroll(int num_lines, bool is_held_key);
    void on_keypress(SDLKey key, bool is_held_key);

public:
    TextView(std::vector<std::string> lines, SystemStyling &sys_styling, TextViewStyling &text_view_styling);
    virtual ~TextView();

    bool render(SDL_Surface *dest_surface, bool force_render) override;
    bool is_done() override;
    void on_keypress(SDLKey key) override;
    void on_keyheld(SDLKey key, uint32_t held_time_ms) override;

    uint32_t get_line_number() const;
    void set_line_number(uint32_t line_number);

    void set_show_title_bar(bool enabled);
    void set_title(std::string title);

    // user tried to scroll up when already at the top
    void set_on_resist_up(std::function<void()> callback);

    // user tried to scroll down when already at the bottom
    void set_on_resist_down(std::function<void()> callback);
};

#endif
