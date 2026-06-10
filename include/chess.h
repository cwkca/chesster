#ifndef CHESS_H
#define CHESS_H

#define NONE 0

typedef enum
{
    KING = 1,
    QUEEN,
    ROOK,
    BISHOP,
    KNIGHT,
    PAWN
} Piece;

typedef enum
{
    P_WHITE = 0,
    P_BLACK = 8
} PieceColor;

typedef enum
{
    WHITE_KING = 1,
    WHITE_QUEEN,
    WHITE_ROOK,
    WHITE_BISHOP,
    WHITE_KNIGHT,
    WHITE_PAWN,
    BLACK_KING = 9,
    BLACK_QUEEN,
    BLACK_ROOK,
    BLACK_BISHOP,
    BLACK_KNIGHT,
    BLACK_PAWN
} ColoredPiece;

typedef char Bitboard[8];

#define COLOR_MASK 8
#define PIECE_MASK 7

extern const char *PIECE_NAMES;
extern const char *COLORED_PIECE_NAMES;

extern const char *STARTING_FEN;

void init_piece_lookup();
char is_color(ColoredPiece piece, PieceColor color);
ColoredPiece get_piece_named(char piece);
int load_fen(const char *fen);
void get_moves(ColoredPiece *board, int square,
               char for_control, Bitboard moves);
void get_all_moves(ColoredPiece *board, PieceColor color,
                   char for_control, Bitboard moves);
void calc_board_overlap(Bitboard a, Bitboard b, Bitboard overlap);

#endif /* CHESS_H */
