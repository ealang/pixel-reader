#ifndef HELD_KEY_TRACKER_H_
#define HELD_KEY_TRACKER_H_

#include <SDL/SDL_keysym.h>

#include <cstdint>
#include <functional>
#include <vector>

class HeldKeyTracker {
    std::vector<SDLKey> keycodes;
    std::vector<uint32_t> held_times;

public:
    HeldKeyTracker(std::vector<SDLKey> keycodes);
    virtual ~HeldKeyTracker();

    void accumulate(uint32_t ms);
    bool for_longest_held(const std::function<void(SDLKey, uint32_t)> &callback);
};

#endif
