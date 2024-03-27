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
    bool rendered = false;
    if (!views.empty())
    {
        auto &top_view = views.back();
        if (top_view != last_top_view.lock())
        {
            top_view->on_focus();
            last_top_view = top_view;
        }

        if (!top_view->is_modal())
        {
            rendered = top_view->render(dest, force_render) || force_render;
        }
        else
        {
            // For modal views, find the last non-modal view and re-render back up to the top
            auto it = views.rbegin();
            while (it != views.rend() && (*it)->is_modal())
            {
                ++it;
            }
            while (it != views.rend() && it != views.rbegin())
            {
                (*it)->render(dest, true);
                --it;
            }
            top_view->render(dest, true);

            rendered = true;
        }
    }
    return rendered;
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

std::shared_ptr<View> ViewStack::top_view() const
{
    if (views.empty())
    {
        return nullptr;
    }
    return views.back();
}
