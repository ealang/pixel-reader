#include "./held_key_tracker.h"

#include <SDL2/SDL.h>

HeldKeyTracker::HeldKeyTracker(std::vector<SDL_Keycode> keycodes)
    : keycodes(keycodes),
      held_times(keycodes.size(), 0)
{
}

HeldKeyTracker::~HeldKeyTracker()
{
}

void HeldKeyTracker::accumulate(uint32_t ms)
{
    // In SDL2, we need to use SDL_GetKeyboardState instead of SDL_GetKeyState
    const Uint8 *keystate = SDL_GetKeyboardState(nullptr);

    auto key_it = keycodes.begin();
    auto time_it = held_times.begin();
    while (key_it != keycodes.end())
    {
        // In SDL2, we need to convert SDL_Keycode to SDL_Scancode using SDL_GetScancodeFromKey
        SDL_Scancode scancode = SDL_GetScancodeFromKey(*key_it);
        if (keystate[scancode])
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

bool HeldKeyTracker::for_longest_held(const std::function<void(SDL_Keycode, uint32_t)> &callback)
{
    uint32_t longest_time = 0;
    SDL_Keycode longest_key = SDLK_UNKNOWN;

    auto key_it = keycodes.begin();
    auto time_it = held_times.begin();
    while (key_it != keycodes.end())
    {
        SDL_Keycode key = *key_it;
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
