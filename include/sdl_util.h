#ifndef SDL_UTIL_H
#define SDL_UTIL_H

#include <SDL.h>

extern SDL_Window *window;
extern SDL_Surface *winSurface;
extern SDL_Surface *piece_imgs[16];

int init_sdl();
void draw_rect(SDL_Rect rect, Uint8 r, Uint8 g, Uint8 b);
void draw_text_centered(const char *text, SDL_Point point);
void cleanup_sdl();

#endif /* SDL_UTIL_H */
