#include <SDL/SDL_keysym.h>

class enum Key {
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_START,
    BUTTON_SELECT,
    BUTTON_L1,
    BUTTON_R1,
    BUTTON_L2,
    BUTTON_R2,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_POWER,
    BUTTON_MENU,
    KEYBOARD_W,
    KEYBOARD_A,
    KEYBOARD_S,
    KEYBOARD_D,
    KEYBOARD_LEFT,
    KEYBOARD_RIGHT,
    KEYBOARD_UP,
    KEYBOARD_DOWN,
    KEYBOARD_SPACE,
    KEYBOARD_RETURN,
    KEYBOARD_ESCAPE
};

#if PLATFORM_MIYOO_MINI

// https://github.com/OnionUI/Onion/blob/main/src/common/system/keymap_sw.h
//   name     sdlk keycode   Key enum code
#define KEYCODE_TABLE {
    {"up",    SDLK_UP,       BUTTON_UP},
    {"down",  SDLK_DOWN,     BUTTON_DOWN},
    {"left",  SDLK_LEFT,     BUTTON_LEFT},
    {"right", SDLK_RIGHT,    BUTTON_RIGHT},
    {"a",     SDLK_SPACE,    BUTTON_A},
}

#define DEFAULT_KEYMAPS {
    {
        "L1/R1",
        {
            "scroll_up": {BUTTON_UP},
            "scroll_down": {BUTTON_DOWN},
            "page_up": {BUTTON_LEFT, BUTTON_L1},
            "page_down": {BUTTON_RIGHT, BUTTON_R1},
            "toggle_statusbar": {BUTTON_A}
        }
    },
    {
        "L2/R2",
        {
            "scroll_up": {BUTTON_UP},
            "scroll_down": {BUTTON_DOWN},
            "page_up": {BUTTON_LEFT, BUTTON_L2},
            "page_down": {BUTTON_RIGHT, BUTTON_R2},
            "toggle_statusbar": {BUTTON_A}
        }
    },
}

#endif

//   name     sdlk keycode   Key enum code
#define KEYCODE_TABLE {
    {"up",    SDLK_UP,       KEYBOARD_UP},
    {"down",  SDLK_DOWN,     KEYBOARD_DOWN},
    {"left",  SDLK_LEFT,     KEYBOARD_LEFT},
    {"right", SDLK_RIGHT,    KEYBOARD_RIGHT},
    {"a",     SDLK_A,        KEYBOARD_A},
}

#define DEFAULT_KEYMAPS {
    {
        "default",
        {
            "scroll_up": {KEYBOARD_UP, KEYBOARD_W},
            "scroll_down": {KEYBOARD_DOWN, KEYBOARD_S},
            "page_up": {KEYBOARD_LEFT, KEYBOARD_A},
            "page_down": {KEYBOARD_RIGHT, KEYBOARD_D},
            "toggle_statusbar": {KEYBOARD_SPACE}
        }
    }
}

#define 


