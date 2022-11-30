#include "./held_key_tracker.h"

#include <SDL/SDL.h>

HeldKeyTracker::HeldKeyTracker(std::vector<SDLKey> keycodes)
    : keycodes(keycodes),
      held_times(keycodes.size(), 0)
{
}

HeldKeyTracker::~HeldKeyTracker()
{
}

void HeldKeyTracker::accumulate(uint32_t ms)
{
    const Uint8 *keystate = SDL_GetKeyState(nullptr);

    auto key_it = keycodes.begin();
    auto time_it = held_times.begin();
    while (key_it != keycodes.end())
    {
        if (keystate[*key_it])
        {
            *time_it += ms;
        }
        else
        {
            *time_it = 0;
        }

        ++key_it;
        ++time_it;
    }
}

bool HeldKeyTracker::for_each_held_key(const std::function<void(SDLKey, uint32_t)> &callback)
{
    bool ran_callbacks = false;

    auto key_it = keycodes.begin();
    auto time_it = held_times.begin();
    while (key_it != keycodes.end())
    {
        uint32_t time = *time_it;
        if (time)
        {
            callback(*key_it, time);
            ran_callbacks = true;
        }

        ++key_it;
        ++time_it;
    }

    return ran_callbacks;
}
