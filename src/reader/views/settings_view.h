#ifndef SETTINGS_VIEW_H_
#define SETTINGS_VIEW_H_

#include "reader/view.h"

#include <string>

struct SystemStyling;
struct TokenViewStyling;

class SettingsView: public View
{
    bool _is_done = false;
    bool needs_render = true;
    uint32_t line_selected = 0;
    std::string font_name;

    SystemStyling &sys_styling;
    TokenViewStyling &token_view_styling;
    uint32_t styling_sub_id;

    int num_menu_items;

    void on_change_theme(int dir);
    void on_change_font_size(int dir);
    void on_change_font_name(int dir);
    void on_change_shoulder_keymap(int dir);
    void on_change_progress();

public:
    SettingsView(
        SystemStyling &sys_styling,
        TokenViewStyling &token_view_styling,
        std::string font_name
    );
    virtual ~SettingsView();

    bool render(SDL_Surface *dest, bool force_render) override;
    bool is_done() override;
    bool is_modal() override;
    void on_keypress(SDLKey key) override;

    void terminate();
    void unterminate();
};

#endif
