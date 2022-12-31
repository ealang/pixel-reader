#ifndef TIMER_H_
#define TIMER_H_

#include <cstdint>

class Timer
{
    uint32_t start_ms;
public:
    Timer();

    void reset();
    uint32_t elapsed_ms() const;
    uint32_t elapsed_sec() const;
};

#endif
