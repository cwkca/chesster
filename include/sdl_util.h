#ifndef SDL_UTIL_H
#define SDL_UTIL_H

#include <SDL.h>

typedef enum { ALIGN_LEFT = 1, ALIGN_CENTER, ALIGN_RIGHT } TextAlign;

extern const SDL_Rect WINDOW_RECT;
extern SDL_Window *window;
extern SDL_Surface *winSurface;
extern SDL_Surface *piece_imgs[16];

int init_sdl();
void draw_rect(SDL_Rect rect, Uint8 r, Uint8 g, Uint8 b);
void draw_text(const char *text, SDL_Color color, SDL_Point point,
               TextAlign alignment);
void cleanup_sdl();

#endif /* SDL_UTIL_H */
