#ifndef VIEW_H_
#define VIEW_H_

#include <SDL/SDL_keysym.h>
#include <SDL/SDL_video.h>

class View
{
public:
    // Returns true if rendering was performed.
    virtual bool render(SDL_Surface *dest, bool force_render) = 0;

    // Return true if the view is no longer needed.
    virtual bool is_done() = 0;

    // If true, the stack will always re-render the view behind this view.
    virtual bool is_modal() { return false; }

    // Key down event.
    virtual void on_keypress(SDLKey key) = 0;

    // Pass key and held time in ms.
    virtual void on_keyheld(SDLKey, uint32_t) {}

    // Called once per main loop iteration regardless of input, with elapsed
    // time since the last call. Used for animations that aren't driven by a
    // held key (e.g. auto-scroll).
    virtual void on_tick(uint32_t) {}

    // If true, the main loop will keep ticking/rendering at full rate even
    // with no input, instead of sleeping between events. Used by views that
    // animate on their own (e.g. auto-scroll).
    virtual bool wants_continuous_render() const { return false; }

    // This view has been popped from the stack (now defunct).
    virtual void on_pop() {}

    // When the view is now on top of the stack.
    virtual void on_focus() {}
};

#endif
