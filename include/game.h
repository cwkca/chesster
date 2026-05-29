#ifndef GAME_H
#define GAME_H

#define NONE 0

typedef enum {
    WHITE = 0,
    BLACK = 8
} PieceColor;

typedef enum {
    KING = 1,
    QUEEN,
    ROOK,
    BISHOP,
    KNIGHT,
    PAWN
} Piece;

typedef enum {
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

#define COLOR_MASK 8
#define PIECE_MASK 7

extern ColoredPiece board[8][8];

int init_game();

#endif /* GAME_H */
