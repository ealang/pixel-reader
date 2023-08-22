#ifndef MATH_H_
#define MATH_H_

inline uint32_t bound(uint32_t val, uint32_t min, uint32_t max)
{
    return std::max(std::min(val, max), min);
}

#endif
