#include "./view_stack.h"


void ViewStack::push(std::shared_ptr<View> view)
{
    views.push_back(view);
}

bool ViewStack::render(SDL_Surface *dest)
{
    if (!views.empty())
    {
        return views.back()->render(dest);
    }
    return false;
}

bool ViewStack::on_keypress(SDLKey key)
{
    if (!views.empty())
    {
        return views.back()->on_keypress(key);
    }
    return false;
}

bool ViewStack::is_done()
{
    while (!views.empty() && views.back()->is_done())
    {
        views.pop_back();
    }

    return views.empty();
}
