/** General graphics and window routines. */

#include <stdio.h>
#include <unistd.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "graphics.h"

SDL_Rect WINDOW_RECT = {100, 100, 800, 600};
const char *FONT_FILES[] = {
    "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/System/Library/Fonts/SFNS.ttf",
    NULL};

SDL_Window *window = NULL;
SDL_Surface *winSurface = NULL;
TTF_Font *font = NULL;

SDL_Window *create_window(const char *title, SDL_Rect rect, Uint32 flags)
{
    return SDL_CreateWindow(title, rect.x, rect.y, rect.w, rect.h, flags);
}

SDL_Window *init_window()
{
    SDL_Window *window = create_window("Chesster", WINDOW_RECT, SDL_WINDOW_SHOWN);
    if (!window)
        return NULL;

    winSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(winSurface, NULL, 0);
    SDL_UpdateWindowSurface(window);

    return window;
}

int init_graphics()
{
    if (TTF_Init() || SDL_Init(SDL_INIT_VIDEO))
        return 1;

    window = init_window();
    if (!window)
    {
        SDL_Quit();
        return 1;
    }

    int i;
    const char *font_file = NULL;
    for (i = 0; FONT_FILES[i]; i++)
        if (access(FONT_FILES[i], F_OK) == 0)
        {
            font_file = FONT_FILES[i];
            break;
        }
    if (!font_file)
    {
        printf("No font file found.\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    font = TTF_OpenFont(font_file, 24);
    if (!font)
    {
        printf("Failed to load font: %s\n", TTF_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    return 0;
}

void draw_rect(SDL_Rect rect, Uint8 r, Uint8 g, Uint8 b)
{
    Uint32 color = SDL_MapRGB(winSurface->format, r, g, b);
    SDL_FillRect(winSurface, &rect, color);
    SDL_UpdateWindowSurface(window);
}

void draw_text(const char *text, TTF_Font *font)
{
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Rect textRect = {20, 20, 0, 0};

    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, textColor);
    SDL_BlitSurface(textSurface, NULL, winSurface, &textRect);
    SDL_FreeSurface(textSurface);

    SDL_UpdateWindowSurface(window);
}
