#ifndef SDL_IMAGE_CACHE_H_
#define SDL_IMAGE_CACHE_H_

#include "./lru_cache.h"
#include "./sdl_pointer.h"

#include <string>

#define IMAGE_CACHE_SIZE_BYTES (64 * 1024 * 1024)

class SDLImageCache
{
    LRUCache<std::string, surface_unique_ptr> cache;
    uint32_t total_size_bytes = 0;

public:
    void put_image(const std::string &key, surface_unique_ptr image);
    SDL_Surface *get_image(const std::string &key);
};

#endif
