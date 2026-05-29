#include <stdio.h>
#include <assert.h>
#include <string.h>

const char *term_pieces[] = {
    " ",
    "♗", "♔", "♘", "♙", "♕", "♖",
    "♝", "♚", "♞", "♟", "♛", "♜"};

const int board[8][8] = {
    12, 9, 7, 11, 8, 7, 9, 12,
    10, 10, 10, 10, 10, 10, 10, 10,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    4, 4, 4, 4, 4, 4, 4, 4,
    6, 3, 1, 5, 2, 1, 3, 6};

#define BUFFER_LEN 20
char piece_str_1[BUFFER_LEN], piece_str_2[BUFFER_LEN];

void format_piece(char *buffer, int rank, int file, char invert)
{
    int piece_index = board[rank][file];
    if (invert)
    {
        if (piece_index)
        {
            if (piece_index > 6)
                piece_index -= 6;
            else
                piece_index += 6;
        }

        sprintf(buffer, "\033[7m%s\033[0m", term_pieces[piece_index]);
    }
    else
    {
        strncpy(buffer, term_pieces[piece_index], BUFFER_LEN);
    }
}

void draw_term_chessboard()
{
    printf("┌───▄▄▄───▄▄▄───▄▄▄───▄▄▄▖\n");

    int rank, file;
    for (rank = 0; rank < 8; rank += 2)
    {
        printf("│");
        for (file = 0; file < 8; file += 2)
        {
            format_piece(piece_str_1, rank, file, 0);
            format_piece(piece_str_2, rank, file + 1, 1);
            printf(" %s █%s█", piece_str_1, piece_str_2);
        }
        printf("▌\n▗▄▄▄▀▀▀▄▄▄▀▀▀▄▄▄▀▀▀▄▄▄▀▀▀▘\n▐");
        for (file = 0; file < 8; file += 2)
        {
            format_piece(piece_str_1, rank + 1, file, 1);
            format_piece(piece_str_2, rank + 1, file + 1, 0);
            printf("█%s█ %s ", piece_str_1, piece_str_2);
        }
        printf("│\n");

        if (rank < 6)
            printf("▝▀▀▀▄▄▄▀▀▀▄▄▄▀▀▀▄▄▄▀▀▀▄▄▄▖\n");
    }
    printf("▝▀▀▀───▀▀▀───▀▀▀───▀▀▀───┘\n");
}
