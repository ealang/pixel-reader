#ifndef SELECTION_MENU_H_
#define SELECTION_MENU_H_

#include "reader/view.h"
#include "util/throttled.h"

#include <SDL/SDL_ttf.h>

#include <functional>
#include <string>
#include <vector>

struct SystemStyling;

class SelectionMenu: public View
{
    bool needs_render = true;

    std::vector<std::string> entries;
    uint32_t cursor_pos = 0;
    uint32_t scroll_pos = 0;
    bool close_on_select = false;

    std::string font;
    SystemStyling &styling;
    const uint32_t styling_sub_id;

    int line_height;
    const int line_padding = 2; // TODO
    const uint32_t num_display_lines;

    Throttled scroll_throttle;

    bool _is_done = false;
    std::function<void(uint32_t)> on_selection;
    std::function<void(uint32_t)> on_focus;
    std::function<void(SDLKey)> default_on_keypress;

    void on_move_down(uint32_t step);
    void on_move_up(uint32_t step);
    void on_select_entry();

public:

    SelectionMenu(SystemStyling &styling, std::string font);
    SelectionMenu(std::vector<std::string> entries, SystemStyling &styling, std::string font);
    virtual ~SelectionMenu();

    void set_entries(std::vector<std::string> new_entries);
    void set_on_selection(std::function<void(uint32_t)> callback);
    void set_on_focus(std::function<void(uint32_t)> callback);
    // Define fallback keypress handler
    void set_default_on_keypress(std::function<void(SDLKey)> callback);
    void set_close_on_select();

    void set_cursor_pos(const std::string &entry);
    void set_cursor_pos(uint32_t pos);

    void close();

    bool render(SDL_Surface *dest_surface, bool force_render) override;
    bool is_done() override;
    void on_keypress(SDLKey key) override;
    void on_keyheld(SDLKey key, uint32_t held_time_ms) override;
};

#endif
