#ifndef SELECTION_MENU_H_
#define SELECTION_MENU_H_

#include "./view.h"

#include <SDL/SDL_ttf.h>

#include <functional>
#include <string>
#include <vector>

class SelectionMenu: public View
{
    std::vector<std::string> entries;
    int cursor_pos = 0;
    int scroll_pos = 0;

    TTF_Font *font;
    int line_height;
    int line_padding = 2;
    int num_display_lines;

    bool _is_done = false;
    std::function<void(std::string)> on_selection;

    void on_move_down();
    void on_move_up();
    void on_select_entry();

public:
    SelectionMenu(std::vector<std::string> entries, TTF_Font *font);
    virtual ~SelectionMenu();

    void set_entries(std::vector<std::string> new_entries);

    bool render(SDL_Surface *dest_surface) override;
    bool on_keypress(SDLKey key) override;
    bool is_done() override;

    void set_on_selection(std::function<void(const std::string &)> on_selection);
};

#endif
