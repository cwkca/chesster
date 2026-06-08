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
    WHITE = 0,
    BLACK = 8
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

extern const char *PIECE_NAMES;
extern const char *COLORED_PIECE_NAMES;

extern const char *STARTING_FEN;

#define COLOR_MASK 8
#define PIECE_MASK 7

void init_piece_lookup();
ColoredPiece get_piece_named(char piece);
int load_fen(const char *fen);

#endif /* CHESS_H */
