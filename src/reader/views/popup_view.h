#ifndef POPUP_VIEW_H_
#define POPUP_VIEW_H_

#include "reader/view.h"

#include <string>

struct SystemStyling;

class PopupView: public View
{
    bool _is_done = false;
    bool _needs_render = true;

    std::string message;
    bool can_close;
    std::string font_name;
    SystemStyling &styling;
    uint32_t styling_sub_id;
public:
    PopupView(const std::string &message, bool can_close, std::string font_name, SystemStyling &styling);
    virtual ~PopupView();

    bool render(SDL_Surface *dest_surface, bool force_render) override;
    bool is_done() override;
    bool is_modal() override;
    void on_keypress(SDLKey key) override;

    void close();
};

#endif
