#ifndef VIEW_STACK_H_
#define VIEW_STACK_H_

#include "./view.h"

#include <memory>
#include <vector>

class ViewStack: public View
{
    std::vector<std::shared_ptr<View>> views;
public:
    void push(std::shared_ptr<View> view);

    bool render(SDL_Surface *dest);
    bool on_keypress(SDLKey key);
    bool is_done();
};

#endif
