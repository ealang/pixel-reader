#ifndef VIEW_H_
#define VIEW_H_

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL.h>

class View
{
public:
    // Returns true if rendering was performed.
    virtual bool render(SDL_Surface *dest_surface, bool force_render) = 0;

    // Return true if the view is no longer needed.
    virtual bool is_done() = 0;

    // If true, the stack will always re-render the view behind this view.
    virtual bool is_modal() { return false; }

    // Key down event.
    virtual void on_keypress(SDL_Keycode key) = 0;

    // Pass key and held time in ms.
    virtual void on_keyheld(SDL_Keycode, uint32_t) {}

    // This view has been popped from the stack (now defunct).
    virtual void on_pop() {}

    // When the view is now on top of the stack.
    virtual void on_focus() {}
};

#endif
