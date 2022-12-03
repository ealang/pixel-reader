#include "./view_stack.h"

void ViewStack::push(std::shared_ptr<View> view)
{
    views.push_back(view);
}

ViewStack::~ViewStack()
{
}

bool ViewStack::render(SDL_Surface *dest, bool force_render)
{
    if (!views.empty())
    {
        return views.back()->render(dest, force_render);
    }
    return false;
}

bool ViewStack::is_done()
{
    return views.empty();
}

void ViewStack::on_keypress(SDLKey key)
{
    if (!views.empty())
    {
        views.back()->on_keypress(key);
    }
}

void ViewStack::on_keyheld(SDLKey key, uint32_t hold_time_ms)
{
    if (!views.empty())
    {
        views.back()->on_keyheld(key, hold_time_ms);
    }
}

bool ViewStack::pop_completed_views()
{
    bool changed_focus = false;
    while (!views.empty() && views.back()->is_done())
    {
        views.back()->on_pop();
        views.pop_back();
        changed_focus = true;
    }

    return changed_focus;
}

void ViewStack::shutdown()
{
    while (!views.empty())
    {
        views.back()->on_pop();
        views.pop_back();
    }
}
