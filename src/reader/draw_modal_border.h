#ifndef DRAW_MODAL_BORDER_H_
#define DRAW_MODAL_BORDER_H_

#include "./color_theme.h"

#include <SDL2/SDL.h>

void draw_modal_border(uint32_t w, uint32_t h, const ColorTheme &theme, SDL_Surface *surface);

#endif
