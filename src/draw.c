/** Game-specific graphics. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "graphics.h"
#include "game.h"

const char *ASSET_PATH = "assets";
#define PATH_MAX 256

const SDL_Rect CHESSBOARD_RECT = {160, 60, 480, 480};
const SDL_Color LIGHT_RGB = {200, 160, 130};
const SDL_Color DARK_RGB = {100, 60, 30};

int square_size;

SDL_Surface *piece_imgs[2][20];

SDL_Surface **get_piece_img(char piece, char color) {
    assert(islower(piece) && islower(color));
    int color_index = color == 'b' ? 1 : 0;
    return &(piece_imgs[color_index][piece - 'a']);
}

int load_pieces(char color) {
    char piece_path[PATH_MAX];
    char piece_code[3];
    const char *piece;

    for (piece = pieces; *piece; piece++) {
        sprintf(piece_path, "%s/%c%c.svg", ASSET_PATH, *piece, color);
        SDL_Surface *piece_img = IMG_Load(piece_path);
        if (!piece_img)
        {
            printf("Failed to load piece %s: %s\n", piece_path, IMG_GetError());
            return 1;
        }
        *get_piece_img(*piece, color) = piece_img;
    }

    return 0;
}

int init_draw()
{
    if (init_graphics()) return 1;
    
    assert(CHESSBOARD_RECT.h == CHESSBOARD_RECT.w);
    assert(CHESSBOARD_RECT.h % 8 == 0);
    square_size = CHESSBOARD_RECT.h / 8;

    if (load_pieces('b') || load_pieces('w')) return 1;

    return 0;
}

void draw_sdl_chessboard()
{
    const Uint32 dark_color = SDL_MapRGB(
        winSurface->format, DARK_RGB.r, DARK_RGB.g, DARK_RGB.b);
    const Uint32 light_color = SDL_MapRGB(
        winSurface->format, LIGHT_RGB.r, LIGHT_RGB.g, LIGHT_RGB.b);

    SDL_Rect square_rect;
    square_rect.y = CHESSBOARD_RECT.y;
    square_rect.h = square_rect.w = square_size;

    int rank, file;
    for (rank = 0; rank < 8; rank++)
    {
        square_rect.x = CHESSBOARD_RECT.x;
        for (file = 0; file < 8; file++)
        {
            Uint32 color = (rank + file) % 2 ? dark_color : light_color;
            SDL_FillRect(winSurface, &square_rect, color);
            square_rect.x += square_size;
        }
        square_rect.y += square_size;
    }

    SDL_UpdateWindowSurface(window);
}

int place_piece(char piece, char color, int file, int rank)
{
    SDL_Rect pieceRect = {
        CHESSBOARD_RECT.x + file * square_size,
        CHESSBOARD_RECT.y + rank * square_size,
        square_size,
        square_size,
    };

    SDL_BlitScaled(*get_piece_img(piece, color), NULL, winSurface, &pieceRect);
    SDL_UpdateWindowSurface(window);

    return 0;
}

int draw_pieces(char *board) {
    int rank, file;
    for (rank = 0; rank < 8; rank++)
        for (file = 0; file < 8; file++) {
            char piece = board[rank * 8 + file];
            if (piece == '.') continue;
            char color = islower(piece) ? 'b' : 'w';
            if (place_piece(tolower(piece), color, file, rank)) return 1;
        }
    return 0;
}

void cleanup_pieces(char color) {
    const char *piece;
    for (piece = pieces; *piece; piece++)
        SDL_FreeSurface(*get_piece_img(*piece, color));
}

void cleanup_draw() {
    cleanup_pieces('b');
    cleanup_pieces('w');

    SDL_DestroyWindow(window);
    SDL_Quit();
}