#include "./timer.h"

Timer::Timer()
    : start(std::chrono::high_resolution_clock::now())
{
}

uint32_t Timer::elapsed_ms() const
{
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}
