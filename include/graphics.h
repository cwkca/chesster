#include <SDL.h>
#include <SDL_ttf.h>

#ifndef GRAPHICS_H
#define GRAPHICS_H

extern SDL_Window *window;
extern SDL_Surface *winSurface;

int init_graphics();
void draw_rect(SDL_Rect rect, Uint8 r, Uint8 g, Uint8 b);
void draw_text(const char *text, TTF_Font *font);

#endif /* GRAPHICS_H */
