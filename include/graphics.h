#include <SDL.h>
#include <SDL_ttf.h>

SDL_Window *window;
SDL_Surface *winSurface;

int init_graphics();
void draw_rect(SDL_Rect rect, Uint8 r, Uint8 g, Uint8 b);
void draw_text(const char *text, TTF_Font *font);
