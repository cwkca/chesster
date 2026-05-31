#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "game.h"

const char *TERM_PIECES[] = {
    " ", "♔", "♕", "♖", "♗", "♘", "♙", " ",
    " ", "♚", "♛", "♜", "♝", "♞", "♟", " "};

#define BUFFER_LEN 20
char inverted_piece[BUFFER_LEN];

void invert_piece(int piece_index)
{
    piece_index ^= COLOR_MASK;
    sprintf(inverted_piece, "\033[7m%s\033[0m", TERM_PIECES[piece_index]);
}

int draw_board_term()
{
    printf("┌───▄▄▄───▄▄▄───▄▄▄───▄▄▄▖\n");

    const char *plain_piece;
    int rank, file, piece_index;
    for (rank = 0; rank < 8; rank += 2)
    {
        printf("│");
        for (file = 0; file < 8; file += 2)
        {
            plain_piece = TERM_PIECES[board[rank][file]];
            invert_piece(board[rank][file + 1]);
            printf(" %s █%s█", plain_piece, inverted_piece);
        }
        printf("▌\n▗▄▄▄▀▀▀▄▄▄▀▀▀▄▄▄▀▀▀▄▄▄▀▀▀▘\n▐");
        for (file = 0; file < 8; file += 2)
        {
            invert_piece(board[rank + 1][file]);
            plain_piece = TERM_PIECES[board[rank + 1][file + 1]];
            printf("█%s█ %s ", inverted_piece, plain_piece);
        }
        printf("│\n");

        if (rank < 6)
            printf("▝▀▀▀▄▄▄▀▀▀▄▄▄▀▀▀▄▄▄▀▀▀▄▄▄▖\n");
    }
    printf("▝▀▀▀───▀▀▀───▀▀▀───▀▀▀───┘\n");

    return 0;
}

int draw_pieces_term()
{
    return 0;
}

void cleanup_term() {}
