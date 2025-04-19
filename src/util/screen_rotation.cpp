#include "screen_rotation.h"
#include "sys/keymap.h"
#include <SDL/SDL.h>

// Define a platform check macro for Miyoo Mini target hardware
#ifdef MIYOO_MINI
  #define REVERSE_ROTATION 1
#else
  #define REVERSE_ROTATION 0
#endif

SDLKey get_rotated_keymap(SDLKey key, ScreenRotation rotation)
{
    // If not a directional key or no rotation, return the original key
    if (rotation == ROTATION_NONE ||
        (key != SW_BTN_UP && key != SW_BTN_DOWN && 
         key != SW_BTN_LEFT && key != SW_BTN_RIGHT)) {
        return key;
    }

    // For target hardware, we need to reverse the rotation direction
    #if REVERSE_ROTATION
    // Reverse the rotation angle
    if (rotation == ROTATION_90) {
        rotation = ROTATION_270;
    } else if (rotation == ROTATION_270) {
        rotation = ROTATION_90;
    }
    #endif

    switch (rotation) {
        case ROTATION_90:
            // 90 degrees clockwise rotation (right becomes down, up becomes right, etc.)
            if (key == SW_BTN_UP) return SW_BTN_RIGHT;
            if (key == SW_BTN_RIGHT) return SW_BTN_DOWN;
            if (key == SW_BTN_DOWN) return SW_BTN_LEFT;
            if (key == SW_BTN_LEFT) return SW_BTN_UP;
            break;

        case ROTATION_180:
            // 180 degrees rotation (up becomes down, left becomes right)
            if (key == SW_BTN_UP) return SW_BTN_DOWN;
            if (key == SW_BTN_RIGHT) return SW_BTN_LEFT;
            if (key == SW_BTN_DOWN) return SW_BTN_UP;
            if (key == SW_BTN_LEFT) return SW_BTN_RIGHT;
            break;

        case ROTATION_270:
            // 270 degrees clockwise rotation (left becomes down, up becomes left, etc.)
            if (key == SW_BTN_UP) return SW_BTN_LEFT;
            if (key == SW_BTN_RIGHT) return SW_BTN_UP;
            if (key == SW_BTN_DOWN) return SW_BTN_RIGHT;
            if (key == SW_BTN_LEFT) return SW_BTN_DOWN;
            break;

        default:
            // Shouldn't get here, but return original key as fallback
            break;
    }

    return key;
}