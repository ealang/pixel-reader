#ifndef FPS_LIMITER_H_
#define FPS_LIMITER_H_

#include <cstdint>

class FPSLimiter
{
    const uint32_t target_delay;
    uint32_t last_time;

public:
    FPSLimiter(float fps);
    void operator()();
};

#endif
