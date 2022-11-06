#include <iostream>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

// from Onion/src/common/system/keymap_sw.h
#define SW_BTN_UP       SDLK_UP
#define SW_BTN_DOWN     SDLK_DOWN
#define SW_BTN_LEFT     SDLK_LEFT
#define SW_BTN_RIGHT    SDLK_RIGHT
#define SW_BTN_A        SDLK_SPACE
#define SW_BTN_B        SDLK_LCTRL
#define SW_BTN_X        SDLK_LSHIFT
#define SW_BTN_Y        SDLK_LALT
#define SW_BTN_L1       SDLK_e
#define SW_BTN_R1       SDLK_t
#define SW_BTN_L2       SDLK_TAB
#define SW_BTN_R2       SDLK_BACKSPACE
#define SW_BTN_SELECT   SDLK_RCTRL
#define SW_BTN_START    SDLK_RETURN
#define SW_BTN_MENU     SDLK_ESCAPE
#define SW_BTN_POWER    SDLK_FIRST

int main () {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    TTF_Init();

    SDL_Surface *video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    SDL_Surface *screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    bool quit = false;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SW_BTN_LEFT:
                            break;
                        case SW_BTN_RIGHT:
                            break;
                        case SW_BTN_A:
                            quit = true;
                            break;
                        case SW_BTN_MENU:
                            quit = true;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }

        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
    }

    SDL_FreeSurface(screen);
    SDL_Quit();
    
    return 0;
}
