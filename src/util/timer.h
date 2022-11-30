#ifndef TIMER_H_
#define TIMER_H_

#include <chrono>

class Timer
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
public:
    Timer();

    uint32_t elapsed_ms() const;
};

#endif
