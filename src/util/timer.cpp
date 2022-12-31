#include "./timer.h"

#include <SDL/SDL.h>

Timer::Timer()
    : start_ms(SDL_GetTicks())
{
}

void Timer::reset()
{
    start_ms = SDL_GetTicks();
}

uint32_t Timer::elapsed_ms() const
{
    return SDL_GetTicks() - start_ms;
}

uint32_t Timer::elapsed_sec() const
{
    return elapsed_ms() / 1000;
}
