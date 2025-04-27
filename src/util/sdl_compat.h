#ifndef SDL_COMPAT_H_
#define SDL_COMPAT_H_

// Compatibility layer for SDL1 -> SDL2 migration
// This helps bridge the gap during partial migration

#include <SDL2/SDL.h>

// Define the old SDL_Key type to be the same as SDL_Keycode
typedef SDL_Keycode SDLKey;

#endif