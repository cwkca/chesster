/** Chess schema and logic. */

#include <assert.h>
#include <stdio.h>
#include <ctype.h>

#include "chess.h"
#include "game.h"

const char *PIECE_NAMES = "_kqrbnp_";
const char *COLORED_PIECE_NAMES = "_KQRBNP__kqrbnp_";

#define LUT_SIZE 50
ColoredPiece PIECE_LOOKUP[LUT_SIZE];

const char *STARTING_FEN = "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr";

/* Private function prototypes */
int safe_get_lut_index(char piece);
void report_fen_error(const char *fen, int index);

void init_piece_lookup()
{
    int lut_index;
    for (lut_index = 0; lut_index < LUT_SIZE; lut_index++)
        PIECE_LOOKUP[lut_index] = NONE;

    int piece_index;
    for (piece_index = 0; piece_index < 16; piece_index++)
    {
        lut_index = safe_get_lut_index(COLORED_PIECE_NAMES[piece_index]);
        assert(lut_index);
        PIECE_LOOKUP[lut_index] = piece_index;
    }
}

ColoredPiece get_piece_named(char piece)
{
    int lut_index = safe_get_lut_index(piece);
    return lut_index ? PIECE_LOOKUP[lut_index] : NONE;
}

int load_fen(const char *fen)
{
    const char *c;
    int rank, file, skip, i;

    rank = file = 0;
    for (c = fen; *c; c++)
    {
        if (isdigit(*c))
        {
            skip = *c - '0';
            for (i = 0; i < skip; i++)
                board[rank][file++] = NONE;
            continue;
        }

        if (*c == '/')
        {
            if (file == 8)
            {
                file = 0;
                rank++;
                continue;
            }
            else
            {
                report_fen_error(fen, c - fen);
                return 1;
            }
        }

        if (file >= 8)
        {
            report_fen_error(fen, c - fen);
            return 1;
        }

        ColoredPiece piece = get_piece_named(*c);
        if (piece)
            board[rank][file++] = piece;
        else
        {
            report_fen_error(fen, c - fen);
            return 1;
        }
    }

    if (file != 8 || ++rank != 8)
    {
        printf("Malformed FEN string:\n    %s\n", fen);
        return 1;
    }

    return 0;
}

/*
 * Private functions
 */

int safe_get_lut_index(char piece)
{
    int index = piece - 'A';
    return (index < 0 || index >= LUT_SIZE)
               ? 0
               : index;
}

void report_fen_error(const char *fen, int index)
{
    printf("Error loading FEN string:\n    %s\n", fen);
    printf("%*s^\n", (int)(4 + index), "");
}
