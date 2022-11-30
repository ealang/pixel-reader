#include "./throttled.h"

Throttled::Throttled(uint32_t init_delay_ms, uint32_t repeat_delay_ms)
    : init_delay_ms(init_delay_ms),
      repeat_delay_ms(repeat_delay_ms),
      last_held_ms(0),
      next_fire_ms(init_delay_ms)
{
}

bool Throttled::operator()(uint32_t held_ms)
{
    bool fire = false;

    if (held_ms <= last_held_ms)
    {
        // reset
        next_fire_ms = init_delay_ms;
    }

    if (held_ms >= next_fire_ms)
    {
        fire = true;
        next_fire_ms = held_ms + repeat_delay_ms;
    }

    last_held_ms = held_ms;
    return fire;
}
