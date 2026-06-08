/** Game-specific graphics. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL.h>

#include "draw_sdl.h"
#include "sdl_util.h"
#include "game.h"
#include "chess.h"

const SDL_Rect CHESSBOARD_RECT = {160, 40, 480, 480};
const SDL_Color LIGHT_RGB = {200, 160, 130};
const SDL_Color DARK_RGB = {100, 60, 30};

int square_size;

/* Private function prototypes */
int place_piece(ColoredPiece piece, int file, int rank);

int init_draw_sdl()
{
    if (init_sdl())
        return 1;

    assert(CHESSBOARD_RECT.h == CHESSBOARD_RECT.w);
    assert(CHESSBOARD_RECT.h % 8 == 0);
    square_size = CHESSBOARD_RECT.h / 8;

    return 0;
}

int draw_board_sdl()
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

    char label[2] = {0};
    SDL_Point text_point;
    text_point.x = CHESSBOARD_RECT.x - (square_size >> 1);
    text_point.y = CHESSBOARD_RECT.y + (square_size >> 1);

    for (rank = 8; rank > 0; rank--)
    {
        *label = rank + '0';
        draw_text_centered(label, text_point);
        text_point.y += square_size;
    }

    text_point.x += square_size;
    for (file = 0; file < 8; file++)
    {
        *label = file + 'a';
        draw_text_centered(label, text_point);
        text_point.x += square_size;
    }

    SDL_UpdateWindowSurface(window);

    return 0;
}

int draw_pieces_sdl()
{
    int rank, file;
    for (rank = 0; rank < 8; rank++)
        for (file = 0; file < 8; file++)
        {
            ColoredPiece piece = board[rank][file];
            if (piece)
                place_piece(piece, file, rank);
        }

    return 0;
}

void cleanup_draw_sdl()
{
    cleanup_sdl();
}

/*
 * Private functions
 */

int place_piece(ColoredPiece piece, int file, int rank)
{
    SDL_Rect pieceRect = {
        CHESSBOARD_RECT.x + file * square_size,
        CHESSBOARD_RECT.y + rank * square_size,
        /* Width and height ignored */
        0,
        0,
    };

    SDL_BlitSurface(piece_imgs[piece], NULL, winSurface, &pieceRect);
    SDL_UpdateWindowSurface(window);

    return 0;
}
