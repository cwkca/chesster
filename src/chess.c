/** Chess schema and logic. */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>

#include "chess.h"
#include "game.h"

const char *PIECE_NAMES = "_kqrbnp_";
const char *COLORED_PIECE_NAMES = "_KQRBNP__kqrbnp_";

#define LUT_SIZE 50
ColoredPiece PIECE_LOOKUP[LUT_SIZE];

const char *STARTING_FEN = "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr";

const char DIRECTIONS[16] = {
    0, -1,
    1, -1,
    1, 0,
    1, 1,
    0, 1,
    -1, 1,
    -1, 0,
    -1, -1};

const char KNIGHT_DIRS[16] = {
    1, -2,
    2, -1,
    2, 1,
    1, 2,
    -1, 2,
    -2, 1,
    -2, -1,
    -1, -2};

typedef enum
{
    NO_CAPTURE = 1,
    CAPTURE_ENEMY,
    CAPTURE_ANY,
    MUST_CAPTURE_ENEMY
} CaptureMode;

/* Private function prototypes */
int safe_get_lut_index(char piece);
void report_fen_error(const char *fen, int index);
void get_slider_moves(
    ColoredPiece *board, int src_square, CaptureMode capture,
    int rank, int file, int dir_offset, int dir_inc, Bitboard moves);
char check_move(ColoredPiece *board, int src_square, CaptureMode capture,
                int rank, int file, Bitboard moves);
char in_bounds(int rank, int file);

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

char is_color(ColoredPiece piece, PieceColor color)
{
    if (!piece)
        return 0;

    PieceColor piece_color = piece & COLOR_MASK;
    return piece_color == color;
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

void find_pieces(ColoredPiece board[8][8], ColoredPiece piece, Vector *squares)
{
    ColoredPiece *board_squares = (ColoredPiece *)board;
    char square;
    for (square = 0; square < 64; square++)
        if (board_squares[square] == piece)
            add_to_vector(squares, &square);
}

void find_file_pawns(ColoredPiece board[8][8], PieceColor color, char file, Vector *squares)
{
    ColoredPiece *board_squares = (ColoredPiece *)board;
    char square;
    for (square = file; square < 64; square += 8)
        if (board_squares[square] == (color | PAWN))
            add_to_vector(squares, &square);
}

void get_moves(ColoredPiece board[8][8], char square,
               char for_control, Bitboard moves)
{
    if (square < 0)
        return;

    ColoredPiece *squares = (ColoredPiece *)board;
    ColoredPiece piece = squares[square];
    PieceColor color = piece & COLOR_MASK;
    char rank = square_rank(square), file = square_file(square);

    CaptureMode capture = for_control ? CAPTURE_ANY : CAPTURE_ENEMY;
    const char *dir;

    int forward;
    char home_row;

    switch (piece & PIECE_MASK)
    {
    case KING:
        for (dir = DIRECTIONS; (dir - DIRECTIONS) < sizeof(DIRECTIONS); dir += 2)
        {
            check_move(squares, square, capture,
                       rank + dir[0], file + dir[1], moves);
        }
        break;

    case QUEEN:
        get_slider_moves(squares, square, capture, rank, file, 0, 1, moves);
        break;

    case ROOK:
        get_slider_moves(squares, square, capture, rank, file, 0, 2, moves);
        break;

    case BISHOP:
        get_slider_moves(squares, square, capture, rank, file, 1, 2, moves);
        break;

    case KNIGHT:
        for (dir = KNIGHT_DIRS; (dir - KNIGHT_DIRS) < sizeof(KNIGHT_DIRS); dir += 2)
        {
            check_move(squares, square, capture,
                       rank + dir[0], file + dir[1], moves);
        }
        break;

    case PAWN:
        if (color == P_WHITE)
        {
            forward = 1;
            home_row = rank == 1;
        }
        else
        {
            forward = -1;
            home_row = rank == 6;
        }

        capture = for_control ? CAPTURE_ANY : MUST_CAPTURE_ENEMY;
        check_move(squares, square, capture, rank + forward, file + 1, moves);
        check_move(squares, square, capture, rank + forward, file - 1, moves);

        if (!for_control)
        {
            check_move(squares, square, NO_CAPTURE, rank + forward, file, moves);
            if (home_row && !board[rank + forward][file])
            {
                check_move(squares, square, NO_CAPTURE,
                           rank + forward + forward, file, moves);
            }
        }

        break;

    default:
        printf("Unrecognized piece %d", piece);
    }
}

void get_all_moves(ColoredPiece board[8][8], PieceColor color,
                   char for_control, Bitboard moves)
{
    clear_board(moves);
    ColoredPiece *squares = (ColoredPiece *)board;

    int sq;
    for (sq = 0; sq < 64; sq++)
        if (is_color(squares[sq], color))
            get_moves(board, sq, for_control, moves);
}

void add_board_to_vector(char src_square, Bitboard move_board, Vector *move_vector)
{
    char rank, square, mask;
    Move move;
    for (rank = square = 0; rank < 8; rank++)
        for (mask = 1; mask; mask <<= 1, square++)
            if (move_board[rank] & mask)
            {
                move = (Move){src_square, square};
                add_to_vector(move_vector, &move);
            }
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

void get_slider_moves(
    ColoredPiece *board, int src_square, CaptureMode capture,
    int rank, int file, int dir_offset, int dir_inc, Bitboard moves)
{
    int r, f;
    const char *dir;

    for (dir = DIRECTIONS + (dir_offset << 1);
         (dir - DIRECTIONS) < sizeof(DIRECTIONS);
         dir += (dir_inc << 1))
    {
        for (r = rank + dir[0], f = file + dir[1];
             check_move(board, src_square, capture, r, f, moves) &&
             !(board[(r << 3) + f]);
             r += dir[0], f += dir[1])
            ;
    }
}

/* Return whether the move is possible. */
char check_move(ColoredPiece *board, int src_square, CaptureMode capture,
                int rank, int file, Bitboard moves)
{
    if (!in_bounds(rank, file))
        return 0;

    ColoredPiece piece = board[src_square];
    PieceColor color = piece & COLOR_MASK;
    ColoredPiece target = board[(rank << 3) + file];
    switch (capture)
    {
    case NO_CAPTURE:
        if (target)
            return 0;
        break;

    case CAPTURE_ENEMY:
        if (is_color(target, color))
            return 0;
        break;

    case CAPTURE_ANY:
        break;

    case MUST_CAPTURE_ENEMY:
        if (!is_color(target, color ^ COLOR_MASK))
            return 0;
        break;

    default:
        printf("Invalid capture mode %d\n", capture);
    }

    /* printf("Found mode %d move to square %d, %d\n", capture, rank, file); */
    moves[rank] |= (1 << file);
    return 1;
}

char in_bounds(int rank, int file)
{
    return rank >= 0 && rank < 8 && file >= 0 && file < 8;
}
