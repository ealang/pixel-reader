#ifndef SELECTION_MENU_H_
#define SELECTION_MENU_H_

#include "reader/view.h"

#include <SDL/SDL_ttf.h>

#include <functional>
#include <string>
#include <vector>

class SelectionMenu: public View
{
    std::vector<std::string> entries;
    uint32_t cursor_pos = 0;
    uint32_t scroll_pos = 0;
    bool close_on_select = false;

    TTF_Font *font;
    int line_height;
    int line_padding = 2; // TODO
    uint32_t num_display_lines;

    bool _is_done = false;
    std::function<void(uint32_t)> on_selection;
    std::function<void(uint32_t)> on_focus;

    void on_move_down(uint32_t step);
    void on_move_up(uint32_t step);
    void on_select_entry();

public:

    SelectionMenu(TTF_Font *font);
    SelectionMenu(std::vector<std::string> entries, TTF_Font *font);
    virtual ~SelectionMenu();

    void set_entries(std::vector<std::string> new_entries);
    void set_on_selection(std::function<void(uint32_t)> callback);
    void set_on_focus(std::function<void(uint32_t)> callback);
    void set_close_on_select();

    void set_cursor_pos(const std::string &entry);
    void set_cursor_pos(uint32_t pos);

    bool render(SDL_Surface *dest_surface) override;
    bool on_keypress(SDLKey key) override;
    bool is_done() override;
};

#endif
