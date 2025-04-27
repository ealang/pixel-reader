#pragma once

#include <SDL2/SDL_keycode.h>

// Screen rotation modes
enum ScreenRotation {
    ROTATION_NONE = 0,      // No rotation
    ROTATION_90 = 90,       // 90 degrees clockwise
    ROTATION_180 = 180,     // 180 degrees (upside down)
    ROTATION_270 = 270      // 270 degrees clockwise (90 counterclockwise)
};

// Get a key mapping based on the screen rotation
// This maps directional keys based on the current rotation
// For example, if the screen is rotated 180 degrees, pressing UP should register as DOWN
SDL_Keycode get_rotated_keymap(SDL_Keycode key, ScreenRotation rotation);