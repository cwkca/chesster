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

const char DIRECTIONS[16] = {0, -1, 1,  -1, 1,  0, 1,  1,
                             0, 1,  -1, 1,  -1, 0, -1, -1};

const char KNIGHT_DIRS[16] = {1,  -2, 2,  -1, 2,  1,  1,  2,
                              -1, 2,  -2, 1,  -2, -1, -1, -2};

typedef enum {
  NO_CAPTURE = 1,
  CAPTURE_ENEMY,
  CAPTURE_ANY,
  MUST_CAPTURE_ENEMY
} CaptureMode;

ByteSet squares;

/* Private function prototypes */
int safe_get_lut_index(char piece);
void report_fen_error(const char *fen, int index);
void get_slider_moves(ColoredPiece *board, int src_square, CaptureMode capture,
                      int rank, int file, int dir_offset, int dir_inc,
                      Bitboard moves);
char check_move(ColoredPiece *board, int src_square, CaptureMode capture,
                int rank, int file, Bitboard moves);
char in_bounds(int rank, int file);
char can_castle(ColoredPiece *board, char row_offset, Bitboard opponent_moves,
                CastleSide side);

void init_chess() {
  int lut_index;
  for (lut_index = 0; lut_index < LUT_SIZE; lut_index++)
    PIECE_LOOKUP[lut_index] = NONE;

  int piece_index;
  for (piece_index = 0; piece_index < 16; piece_index++) {
    lut_index = safe_get_lut_index(COLORED_PIECE_NAMES[piece_index]);
    assert(lut_index);
    PIECE_LOOKUP[lut_index] = piece_index;
  }

  init_set(&squares, 16);
}

void cleanup_chess() { cleanup_set(&squares); }

char is_color(ColoredPiece piece, PieceColor color) {
  if (!piece)
    return 0;

  PieceColor piece_color = piece & COLOR_MASK;
  return piece_color == color;
}

ColoredPiece get_piece_named(char piece) {
  int lut_index = safe_get_lut_index(piece);
  return lut_index ? PIECE_LOOKUP[lut_index] : NONE;
}

int load_fen(const char *fen) {
  const char *c;
  int rank, file, skip, i;

  rank = file = 0;
  for (c = fen; *c; c++) {
    if (isdigit(*c)) {
      skip = *c - '0';
      for (i = 0; i < skip; i++)
        board[(rank << 3) + file++] = NONE;
      continue;
    }

    if (*c == '/') {
      if (file == 8) {
        file = 0;
        rank++;
        continue;
      } else {
        report_fen_error(fen, c - fen);
        return 1;
      }
    }

    if (file >= 8) {
      report_fen_error(fen, c - fen);
      return 1;
    }

    ColoredPiece piece = get_piece_named(*c);
    if (piece)
      board[(rank << 3) + file++] = piece;
    else {
      report_fen_error(fen, c - fen);
      return 1;
    }
  }

  if (file != 8 || ++rank != 8) {
    printf("Malformed FEN string:\n    %s\n", fen);
    return 1;
  }

  return 0;
}

void find_all_pieces(ColoredPiece *board, PieceColor color, ByteSet *squares) {
  char square;
  for (square = 0; square < 64; square++)
    if ((board[square] & COLOR_MASK) == color)
      set_add(squares, square);
}

void find_pieces(ColoredPiece *board, ColoredPiece piece, ByteSet *squares) {
  char square;
  for (square = 0; square < 64; square++)
    if (board[square] == piece)
      set_add(squares, square);
}

void find_file_pawns(ColoredPiece *board, PieceColor color, char file,
                     ByteSet *squares) {
  char square;
  for (square = file; square < 64; square += 8)
    if (board[square] == (color | PAWN))
      set_add(squares, square);
}

void get_moves(ColoredPiece *board, char square, char for_control,
               Bitboard moves) {
  assert(square >= 0);

  ColoredPiece piece = board[square];
  PieceColor color = piece & COLOR_MASK;
  char rank = square_rank(square), file = square_file(square);

  CaptureMode capture = for_control ? CAPTURE_ANY : CAPTURE_ENEMY;
  const char *dir;

  int forward;
  char home_row;

  switch (piece & PIECE_MASK) {
  case KING:
    for (dir = DIRECTIONS; (dir - DIRECTIONS) < sizeof(DIRECTIONS); dir += 2)
      check_move(board, square, capture, rank + dir[0], file + dir[1], moves);
    break;

  case QUEEN:
    get_slider_moves(board, square, capture, rank, file, 0, 1, moves);
    break;

  case ROOK:
    get_slider_moves(board, square, capture, rank, file, 0, 2, moves);
    break;

  case BISHOP:
    get_slider_moves(board, square, capture, rank, file, 1, 2, moves);
    break;

  case KNIGHT:
    for (dir = KNIGHT_DIRS; (dir - KNIGHT_DIRS) < sizeof(KNIGHT_DIRS);
         dir += 2) {
      check_move(board, square, capture, rank + dir[0], file + dir[1], moves);
    }
    break;

  case PAWN:
    if (color == P_WHITE) {
      forward = 1;
      home_row = rank == 1;
    } else {
      forward = -1;
      home_row = rank == 6;
    }

    capture = for_control ? CAPTURE_ANY : MUST_CAPTURE_ENEMY;
    check_move(board, square, capture, rank + forward, file + 1, moves);
    check_move(board, square, capture, rank + forward, file - 1, moves);

    if (!for_control) {
      check_move(board, square, NO_CAPTURE, rank + forward, file, moves);
      if (home_row && !board[((rank + forward) << 3) + file])
        check_move(board, square, NO_CAPTURE, rank + forward + forward, file,
                   moves);
    }

    break;

  default:
    printf("Unrecognized piece %d", piece);
  }
}

void get_moves_from(ColoredPiece *board, ByteSet *src_squares, Vector *moves) {
  Bitboard bit_moves;
  char i, square;

  for (i = 0; i < src_squares->size; i++) {
    square = src_squares->bytes[i];
    clear_board(bit_moves);
    get_moves(board, square, 0, bit_moves);
    add_board_to_vector(square, bit_moves, moves);
  }
}

void get_all_moves(ColoredPiece *board, PieceColor color, char for_control,
                   Bitboard moves) {
  clear_board(moves);

  int sq;
  for (sq = 0; sq < 64; sq++)
    if (is_color(board[sq], color))
      get_moves(board, sq, for_control, moves);
}

void get_castles(ColoredPiece *board, PieceColor color, Vector *moves) {
  Bitboard opponent_moves;
  Move castle;
  char king_square = 4, rights = board[64];
  char row_offset = color == P_BLACK ? 56 : 0;

  if (color == P_BLACK)
    rights >>= 2;
  if (!(rights & 3))
    return;

  get_all_moves(board, color ^ COLOR_MASK, 1, opponent_moves);
  castle.src_square = (color == P_BLACK ? 56 : 4) | SPECIAL_MOVE;

  if ((rights & CASTLE_QUEENSIDE) &&
      can_castle(board, row_offset, opponent_moves, CASTLE_QUEENSIDE)) {
    castle.dest_square = row_offset;
    vector_append(moves, &castle);
  }

  if ((rights & CASTLE_KINGSIDE) &&
      can_castle(board, row_offset, opponent_moves, CASTLE_KINGSIDE)) {
    castle.dest_square = row_offset + 7;
    vector_append(moves, &castle);
  }
}

void parse_castle(Move *m, Castle *c) {
  assert(is_castle(m));

  c->king_start = m->src_square & 0x3F;
  c->rook_start = m->dest_square;
  c->dir = c->king_start > c->rook_start ? -1 : 1;

  c->king_end = c->king_start + (c->dir << 1);
  c->rook_end = c->king_start + c->dir;
}

void update_castling_rights(char start_square) {
  switch (start_square) {
  case 0:
    board[64] &= ~CASTLE_WQ;
    break;

  case 4:
    board[64] &= ~CASTLE_W;
    break;

  case 7:
    board[64] &= ~CASTLE_WK;
    break;

  case 56:
    board[64] &= ~CASTLE_BQ;
    break;

  case 60:
    board[64] &= ~CASTLE_B;
    break;

  case 63:
    board[64] &= ~CASTLE_BK;
  }
}

char king_in_check(ColoredPiece *board, PieceColor color) {
  char king_square;
  Bitboard opponent_moves;

  clear_set(&squares);
  find_pieces(board, KING | color, &squares);
  assert(squares.size == 1);
  king_square = squares.bytes[0];
  clear_set(&squares);

  get_all_moves(board, color ^ COLOR_MASK, 1, opponent_moves);
  return has_square(opponent_moves, king_square);
}

void add_board_to_vector(char src_square, Bitboard move_board,
                         Vector *move_vector) {
  char rank, square, mask;
  Move move;
  for (rank = square = 0; rank < 8; rank++)
    for (mask = 1; mask; mask <<= 1, square++)
      if (move_board[rank] & mask) {
        move = (Move){src_square, square};
        vector_append(move_vector, &move);
      }
}

/*
 * Private functions
 */

int safe_get_lut_index(char piece) {
  int index = piece - 'A';
  return (index < 0 || index >= LUT_SIZE) ? 0 : index;
}

void report_fen_error(const char *fen, int index) {
  printf("Error loading FEN string:\n    %s\n", fen);
  printf("%*s^\n", (int)(4 + index), "");
}

void get_slider_moves(ColoredPiece *board, int src_square, CaptureMode capture,
                      int rank, int file, int dir_offset, int dir_inc,
                      Bitboard moves) {
  int r, f;
  const char *dir;

  for (dir = DIRECTIONS + (dir_offset << 1);
       (dir - DIRECTIONS) < sizeof(DIRECTIONS); dir += (dir_inc << 1)) {
    for (r = rank + dir[0], f = file + dir[1];
         check_move(board, src_square, capture, r, f, moves) &&
         !(board[(r << 3) + f]);
         r += dir[0], f += dir[1])
      ;
  }
}

/* Return whether the move is possible. */
char check_move(ColoredPiece *board, int src_square, CaptureMode capture,
                int rank, int file, Bitboard moves) {
  if (!in_bounds(rank, file))
    return 0;

  ColoredPiece piece = board[src_square];
  PieceColor color = piece & COLOR_MASK;
  ColoredPiece target = board[(rank << 3) + file];
  switch (capture) {
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

  moves[rank] |= (1 << file);
  return 1;
}

char in_bounds(int rank, int file) {
  return rank >= 0 && rank < 8 && file >= 0 && file < 8;
}

char can_castle(ColoredPiece *board, char row_offset, Bitboard opponent_moves,
                CastleSide side) {
  char sq;
  switch (side) {
  case CASTLE_QUEENSIDE:
    // Verify vacant squares
    for (sq = 1; sq < 4; sq++)
      if (board[row_offset + sq])
        return 0;

    // Verify king not in check
    for (sq = 2; sq < 5; sq++)
      if (has_square(opponent_moves, row_offset + sq))
        return 0;

    return 1;

  case CASTLE_KINGSIDE:
    // Verify vacant squares
    for (sq = 5; sq < 7; sq++)
      if (board[row_offset + sq])
        return 0;

    // Verify king not in check
    for (sq = 4; sq < 7; sq++)
      if (has_square(opponent_moves, row_offset + sq))
        return 0;

    return 1;
  }
}
