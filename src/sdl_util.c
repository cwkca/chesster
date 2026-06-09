/** General SDL drawing and window routines. */

#include <stdio.h>
#include <unistd.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "sdl_util.h"
#include "chess.h"
#include "draw.h"

SDL_Rect WINDOW_RECT = {100, 100, 800, 600};
const char *PIECE_IMG_PATH = "assets/pieces/png60";
const char *FONT_PATH = "assets/fonts/sans.ttf";
#define PATH_MAX 256

SDL_Window *window = NULL;
SDL_Surface *winSurface = NULL;
SDL_Surface *piece_imgs[16];
TTF_Font *font = NULL;

/* Private function prototypes */
SDL_Window *create_window(const char *title, SDL_Rect rect, Uint32 flags);
SDL_Window *init_window();
int init_fonts();
int load_pieces(PieceColor color);
void cleanup_pieces();

int init_sdl()
{
    if (SDL_Init(SDL_INIT_VIDEO))
        return 1;

    window = init_window();
    if (!window)
    {
        SDL_Quit();
        return 1;
    }

    if (init_fonts())
        return 1;

    if (load_pieces(P_WHITE) || load_pieces(P_BLACK))
        return 1;

    return 0;
}

void draw_rect(SDL_Rect rect, Uint8 r, Uint8 g, Uint8 b)
{
    Uint32 color = SDL_MapRGB(winSurface->format, r, g, b);
    SDL_FillRect(winSurface, &rect, color);
    SDL_UpdateWindowSurface(window);
}

void draw_text_centered(const char *text, SDL_Point point)
{
    SDL_Rect textRect = {0};

    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, C_WHITE);

    textRect.x = point.x - (textSurface->w >> 1);
    textRect.y = point.y - (textSurface->h >> 1);

    SDL_BlitSurface(textSurface, NULL, winSurface, &textRect);
    SDL_FreeSurface(textSurface);

    SDL_UpdateWindowSurface(window);
}

void cleanup_sdl()
{
    cleanup_pieces();
    SDL_DestroyWindow(window);
    SDL_Quit();
}

/*
 * Private functions
 */

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

int init_fonts()
{
    if (TTF_Init())
        return 1;

    font = TTF_OpenFont(FONT_PATH, 24);
    if (!font)
    {
        printf("Failed to load font: %s\n", TTF_GetError());
        return 1;
    }

    return 0;
}

int load_pieces(PieceColor color)
{
    Piece piece;
    char piece_path[PATH_MAX];
    char color_code = color == P_WHITE ? 'w' : 'b';

    for (piece = KING; piece <= PAWN; piece++)
    {
        sprintf(piece_path, "%s/%c%c.png",
                PIECE_IMG_PATH, PIECE_NAMES[piece], color_code);
        SDL_Surface *piece_img = IMG_Load(piece_path);
        if (!piece_img)
        {
            printf("Failed to load piece %s: %s\n", piece_path, IMG_GetError());
            return 1;
        }
        piece_imgs[color | piece] = piece_img;
    }

    return 0;
}

void cleanup_pieces()
{
    Piece piece;
    for (piece = KING; piece <= PAWN; piece++)
    {
        SDL_FreeSurface(piece_imgs[P_WHITE | piece]);
        SDL_FreeSurface(piece_imgs[P_BLACK | piece]);
    }
}
