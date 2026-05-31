/** Game-specific graphics. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "draw_sdl.h"
#include "sdl_util.h"
#include "game.h"
#include "chess.h"

const char *PIECE_IMG_PATH = "pieces/png60";
#define PATH_MAX 256

const SDL_Rect CHESSBOARD_RECT = {160, 60, 480, 480};
const SDL_Color LIGHT_RGB = {200, 160, 130};
const SDL_Color DARK_RGB = {100, 60, 30};

int square_size;

SDL_Surface *piece_imgs[16];

/* Private function prototypes */
int load_pieces(PieceColor color);
int place_piece(ColoredPiece piece, int file, int rank);
void cleanup_pieces();

int init_draw_sdl()
{
    if (init_sdl())
        return 1;

    assert(CHESSBOARD_RECT.h == CHESSBOARD_RECT.w);
    assert(CHESSBOARD_RECT.h % 8 == 0);
    square_size = CHESSBOARD_RECT.h / 8;

    if (load_pieces(WHITE) || load_pieces(BLACK))
        return 1;

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

void cleanup_sdl()
{
    cleanup_pieces();
    SDL_DestroyWindow(window);
    SDL_Quit();
}

/*
 * Private functions
 */

int load_pieces(PieceColor color)
{
    Piece piece;
    char piece_path[PATH_MAX];
    char color_code = color == WHITE ? 'w' : 'b';

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

void cleanup_pieces()
{
    Piece piece;
    for (piece = KING; piece <= PAWN; piece++)
    {
        SDL_FreeSurface(piece_imgs[WHITE | piece]);
        SDL_FreeSurface(piece_imgs[BLACK | piece]);
    }
}
