#ifndef SCREEN_H_
#define SCREEN_H_

extern short unsigned int SCREEN_WIDTH;
extern short unsigned int SCREEN_HEIGHT;

struct SDL_PixelFormat;
SDL_PixelFormat *get_render_surface_format();
void set_render_surface_format(SDL_PixelFormat *format);

#endif
