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

bool HeldKeyTracker::for_longest_held(const std::function<void(SDLKey, uint32_t)> &callback)
{
    uint32_t longest_time = 0;
    SDLKey longest_key = SDLK_UNKNOWN;

    auto key_it = keycodes.begin();
    auto time_it = held_times.begin();
    while (key_it != keycodes.end())
    {
        SDLKey key = *key_it;
        uint32_t time = *time_it;
        if (time > longest_time)
        {
            longest_time = time;
            longest_key = key;
        }

        ++key_it;
        ++time_it;
    }

    if (longest_time != 0)
    {
        callback(longest_key, longest_time);
        return true;
    }

    return false;
}
