#ifndef SDL_UTIL_H
#define SDL_UTIL_H

#include <SDL.h>
#include <SDL_ttf.h>

extern SDL_Window *window;
extern SDL_Surface *winSurface;

int init_sdl();
void draw_rect(SDL_Rect rect, Uint8 r, Uint8 g, Uint8 b);
void draw_text(const char *text, TTF_Font *font);

#endif /* SDL_UTIL_H */
