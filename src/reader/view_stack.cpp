#include "./view_stack.h"

void ViewStack::push(std::shared_ptr<View> view)
{
    if (!views.empty())
    {
        views.back()->on_lose_focus();
    }
    view->on_gain_focus();
    views.push_back(view);
}

ViewStack::~ViewStack()
{
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
    return views.empty();
}

void ViewStack::pop_completed_views()
{
    bool changed_focus = false;
    while (!views.empty() && views.back()->is_done())
    {
        views.back()->on_pop();
        views.pop_back();
        changed_focus = true;
    }

    if (!views.empty())
    {
        if (changed_focus)
        {
            views.back()->on_gain_focus();
        }
    }
}

void ViewStack::shutdown()
{
    while (!views.empty())
    {
        views.back()->on_pop();
        views.pop_back();
    }
}
