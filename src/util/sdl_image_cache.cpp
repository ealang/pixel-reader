#include "./sdl_image_cache.h"

namespace
{

uint32_t surface_size_bytes(const SDL_Surface *surface)
{
    return surface->pitch * surface->h;
}

} // namespace

void SDLImageCache::put_image(const std::string &key, surface_unique_ptr image)
{
    uint32_t surface_size = surface_size_bytes(image.get());

    while (cache.size() && total_size_bytes + surface_size > IMAGE_CACHE_SIZE_BYTES)
    {
        total_size_bytes -= surface_size_bytes(
            cache.back_value().get()
        );
        cache.pop();
    }

    cache.put(key, std::move(image));
    total_size_bytes += surface_size;
}

SDL_Surface *SDLImageCache::get_image(const std::string &key)
{
    if (!cache.has(key))
    {
        return nullptr;
    }
    return cache[key].get();
}
