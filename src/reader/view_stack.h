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
    virtual ~ViewStack();

    bool render(SDL_Surface *dest) override;
    bool is_done() override;

    void on_keypress(SDLKey key) override;
    void on_keyheld(SDLKey key, uint32_t hold_time_ms) override;

    // Pop views that report as done.
    void pop_completed_views();
    // Pop all views.
    void shutdown();
};

#endif
