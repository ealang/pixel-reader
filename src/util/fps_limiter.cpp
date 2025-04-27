#include "./fps_limiter.h"

#include <SDL2/SDL.h>

FPSLimiter::FPSLimiter(float fps)
    : target_delay(static_cast<uint32_t>(1000 / fps)),
      last_time(SDL_GetTicks())
{
}

void FPSLimiter::operator()()
{
    uint32_t cur_time = SDL_GetTicks();
    uint32_t target_time = last_time + target_delay;

    if (cur_time < target_time)
    {
        SDL_Delay(target_time - cur_time);
    }
    last_time = SDL_GetTicks();
}
