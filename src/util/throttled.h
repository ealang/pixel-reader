#ifndef THROTTLED_H_
#define THROTTLED_H_

#include <cstdint>

class Throttled
{
    const uint32_t init_delay_ms;
    const uint32_t repeat_delay_ms;

    uint32_t last_held_ms;
    uint32_t next_fire_ms;

public:

    Throttled(uint32_t init_delay_ms, uint32_t repeat_delay_ms);
    bool operator()(uint32_t held_ms);
};

#endif
