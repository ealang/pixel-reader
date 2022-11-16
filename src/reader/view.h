#ifndef VIEW_H_
#define VIEW_H_

#include <SDL/SDL_video.h>
#include <SDL/SDL_keysym.h>

class View
{
public:
    // Returns true if rendering was performed.
    virtual bool render(SDL_Surface *dest) = 0;

    // Return true if the event was handled.
    virtual bool on_keypress(SDLKey key) = 0;

    // Return true if the view is no longer needed.
    virtual bool is_done() = 0;

    virtual void on_lose_focus() {}
    virtual void on_gain_focus() {}
};

#endif
