#ifndef CHESS_H
#define CHESS_H

#include "data.h"

#define NONE 0

typedef enum { KING = 1, QUEEN, ROOK, BISHOP, KNIGHT, PAWN } Piece;

typedef enum { P_WHITE = 0, P_BLACK = 8 } PieceColor;

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

#define COLOR_MASK 8
#define PIECE_MASK 7

typedef enum { CASTLE_QUEENSIDE = 1, CASTLE_KINGSIDE = 2 } CastleSide;

#define CASTLE_WQ CASTLE_QUEENSIDE
#define CASTLE_WK CASTLE_KINGSIDE
#define CASTLE_W (CASTLE_WQ | CASTLE_WK)
#define CASTLE_BQ (CASTLE_QUEENSIDE << 2)
#define CASTLE_BK (CASTLE_KINGSIDE << 2)
#define CASTLE_B (CASTLE_BQ | CASTLE_BK)
#define CASTLE_ALL (CASTLE_W | CASTLE_B)

typedef struct {
  char king_start, rook_start, king_end, rook_end, dir;
} Castle;

extern const char *PIECE_NAMES, *COLORED_PIECE_NAMES, *STARTING_FEN;
extern const int MATERIAL_VALUES[7];

void init_chess();
void cleanup_chess();

#define is_rank(x) ((x) > '0' && (x) < '9')
#define is_file(x) ((x) >= 'a' && (x) <= 'h')
#define MOVE_CHARS "12345678abcdefghxo"
#define is_move_char(x) (strchr(MOVE_CHARS, (x)))

char is_color(ColoredPiece piece, PieceColor color);
ColoredPiece get_piece_named(char piece);

int load_fen(const char *fen);

void find_all_pieces(ColoredPiece *board, PieceColor color, ByteSet *squares);
void find_pieces(ColoredPiece *board, ColoredPiece piece, ByteSet *squares);
void find_file_pawns(ColoredPiece *board, PieceColor color, char file,
                     ByteSet *squares);

void get_moves(ColoredPiece *board, char square, char for_control,
               Bitboard moves);
void get_moves_from(ColoredPiece *board, ByteSet *src_squares, Vector *moves);
void get_all_moves(ColoredPiece *board, PieceColor color, char for_control,
                   Bitboard moves);

void get_castles(ColoredPiece *board, PieceColor color, Vector *moves);
void parse_castle(Move *move, Castle *castle);
void update_castling_rights(char start_square);
#define is_castle(move) ((move)->src_square & SPECIAL_MOVE)

char king_in_check(ColoredPiece *board, PieceColor color);

void add_board_to_vector(char src_square, Bitboard move_board,
                         Vector *move_vector);

#endif /* CHESS_H */
