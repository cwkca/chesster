/** Game logic. */

#include <stddef.h>

#include "game.h"
#include "draw.h"

const char *PIECE_NAMES = " kqrbnp ";
const char *COLORED_PIECE_NAMES = " KQRBNP  kqrbnp ";

ColoredPiece board[8][8];

DrawAdapter *draw = NULL;

// Todo: FEN
void setup_board()
{
    board[0][0] = BLACK | ROOK;
    board[0][1] = BLACK | KNIGHT;
    board[0][2] = BLACK | BISHOP;
    board[0][3] = BLACK | QUEEN;
    board[0][4] = BLACK | KING;
    board[0][5] = BLACK | BISHOP;
    board[0][6] = BLACK | KNIGHT;
    board[0][7] = BLACK | ROOK;

    int file;
    for (file = 0; file < 8; file++)
        board[1][file] = BLACK | PAWN;

    int rank;
    for (rank = 2; rank < 6; rank++)
        for (file = 0; file < 8; file++)
            board[rank][file] = NONE;

    for (file = 0; file < 8; file++)
        board[6][file] = WHITE | PAWN;

    board[7][0] = WHITE | ROOK;
    board[7][1] = WHITE | KNIGHT;
    board[7][2] = WHITE | BISHOP;
    board[7][3] = WHITE | QUEEN;
    board[7][4] = WHITE | KING;
    board[7][5] = WHITE | BISHOP;
    board[7][6] = WHITE | KNIGHT;
    board[7][7] = WHITE | ROOK;
}

int init_game()
{
    draw = init_draw();
    if (!draw)
        return 1;

    setup_board();

    if (draw->draw_board() || draw->draw_pieces())
        return 1;

    return 0;
}

void cleanup_game()
{
    if (draw)
        draw->cleanup();
}
