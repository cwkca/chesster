/** Chess schema and logic. */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>

#include "chess.h"
#include "debug.h"
#include "game.h"

const char *PIECE_NAMES = "_kqrbnp_";
const char *COLORED_PIECE_NAMES = "_KQRBNP__kqrbnp_";

#define LUT_SIZE 50
ColoredPiece PIECE_LOOKUP[LUT_SIZE];

const char *STARTING_FEN = "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr";

const char MATERIAL_VALUES[7] = {0, 0, 9, 5, 3, 3, 1};

const char DIRECTIONS[16] = {0, -1, 1,  -1, 1,  0, 1,  1,
                             0, 1,  -1, 1,  -1, 0, -1, -1};

const char KNIGHT_DIRS[16] = {1,  -2, 2,  -1, 2,  1,  1,  2,
                              -1, 2,  -2, 1,  -2, -1, -1, -2};

const char PERIMETER[32] = {0,  2,  1,  2, 2,  2, 2,  1,  2,  0,  2,
                            -1, 2,  -2, 1, -2, 0, -2, -1, -2, -2, -2,
                            -2, -1, -2, 0, -2, 1, -2, 2,  -1, 2};

typedef enum {
  NO_CAPTURE = 1,
  CAPTURE_ENEMY,
  CAPTURE_ANY,
  MUST_CAPTURE_ENEMY
} CaptureMode;

ByteSet squares;
Vector move_searches, temp_moves;

/* Private function prototypes */
int safe_get_lut_index(char piece);
void report_fen_error(const char *fen, int index);
void get_slider_moves(const ColoredPiece *board, int src_square,
                      CaptureMode capture, int rank, int file, int dir_offset,
                      int dir_inc, Bitboard moves);
char check_move(const ColoredPiece *board, int src_square, CaptureMode capture,
                int rank, int file, Bitboard moves);
char can_castle(const ColoredPiece *board, char row_offset,
                Bitboard opponent_moves, CastleSide side);

void init_chess(char search_depth) {
  int lut_index, piece_index, vec;

  for (lut_index = 0; lut_index < LUT_SIZE; lut_index++)
    PIECE_LOOKUP[lut_index] = NONE;

  for (piece_index = 0; piece_index < 16; piece_index++) {
    lut_index = safe_get_lut_index(COLORED_PIECE_NAMES[piece_index]);
    assert(lut_index);
    PIECE_LOOKUP[lut_index] = piece_index;
  }

  set_init(&squares, 16);
  vector_init(&move_searches, sizeof(Vector), search_depth + 1);
  move_searches.size = search_depth + 1;
  for (vec = 0; vec < search_depth + 1; vec++)
    vector_init(vector_get(&move_searches, vec), sizeof(Move),
                MOVE_VECTOR_SIZE);

  vector_init(&temp_moves, sizeof(Move), MOVE_VECTOR_SIZE);
}

void cleanup_chess() {
  int v;

  set_cleanup(&squares);

  for (v = 0; v < move_searches.size; v++)
    vector_cleanup(vector_get(&move_searches, v));
  vector_cleanup(&move_searches);

  vector_cleanup(&temp_moves);
}

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

void find_all_pieces(const ColoredPiece *board, PieceColor color,
                     ByteSet *squares) {
  char square;
  for (square = 0; square < 64; square++)
    if (is_color(board[square], color))
      set_add(squares, square);
}

void find_pieces(const ColoredPiece *board, ColoredPiece piece,
                 ByteSet *squares) {
  char square;
  for (square = 0; square < 64; square++)
    if (board[square] == piece)
      set_add(squares, square);
}

void find_file_pawns(const ColoredPiece *board, PieceColor color, char file,
                     ByteSet *squares) {
  char square;
  for (square = file; square < 64; square += 8)
    if (board[square] == (color | PAWN))
      set_add(squares, square);
}

char in_bounds(char rank, char file) {
  return rank >= 0 && rank < 8 && file >= 0 && file < 8;
}

void get_moves(const ColoredPiece *board, char square, char for_control,
               Bitboard moves) {
  assert(square >= 0);

  ColoredPiece piece = board[square];
  assert(piece);
  PieceColor color = piece & COLOR_MASK;
  char rank = square_rank(square), file = square_file(square);

  CaptureMode capture = for_control ? CAPTURE_ANY : CAPTURE_ENEMY;
  const char *dir;

  int forward;
  char home_row;

  switch (piece & PIECE_MASK) {
  case KING:
    for (dir = DIRECTIONS; (dir - DIRECTIONS) < 16; dir += 2)
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
    throw("Unrecognized piece");
  }
}

void get_moves_from(const ColoredPiece *board, ByteSet *squares,
                    Vector *moves) {
  Bitboard bit_moves;
  char i, square;

  for (i = 0; i < squares->size; i++) {
    square = squares->bytes[i];
    clear_board(bit_moves);
    get_moves(board, square, 0, bit_moves);
    add_board_to_vector(square, bit_moves, moves);
  }
}

void get_all_moves(const ColoredPiece *board, PieceColor color,
                   char for_control, Bitboard moves) {
  char sq;
  clear_board(moves);
  for (sq = 0; sq < 64; sq++)
    if (is_color(board[sq], color))
      get_moves(board, sq, for_control, moves);
}

void get_legal_moves(ColoredPiece *board, PieceColor color, Vector *moves) {
  set_clear(&squares);
  vector_clear(moves);

  find_all_pieces(board, color, &squares);
  assert(squares.size);

  get_moves_from(board, &squares, moves);
  get_castles(board, color, moves);
  filter_check(board, color, moves);
}

void do_move(ColoredPiece *board, Move *move, Piece promote, char testing) {
  char king, rook, dir, promo_rank;
  ColoredPiece piece;
  PieceColor color;
  Castle c;

  if (is_castle(move)) {
    parse_castle(move, &c);
    board[c.king_end] = board[c.king_start];
    board[c.rook_end] = board[c.rook_start];
    board[c.king_start] = board[c.rook_start] = NONE;
  } else {
    // Handle normal moves
    piece = board[move->src_square];
    color = piece & COLOR_MASK;
    board[move->src_square] = NONE;
    board[move->dest_square] = piece;

    promo_rank = color == P_WHITE ? 7 : 0;
    if (promote && (piece & PIECE_MASK) == PAWN &&
        square_rank(move->dest_square) == promo_rank)
      board[move->dest_square] = promote | color;
  }

  if (!testing)
    update_castling_rights(move->src_square & 0x3F, board + 64);
}

void undo_move(ColoredPiece *board, Move *move, ColoredPiece captured) {
  Castle c;

  if (is_castle(move)) {
    parse_castle(move, &c);
    board[c.king_start] = board[c.king_end];
    board[c.rook_start] = board[c.rook_end];
    board[c.king_end] = board[c.rook_end] = NONE;
  } else {
    board[move->src_square] = board[move->dest_square];
    board[move->dest_square] = captured;
  }
}

void get_castles(const ColoredPiece *board, PieceColor color, Vector *moves) {
  Bitboard opponent_moves;
  Move castle;
  char king_square = 4, rights = board[64];
  char row_offset = color == P_BLACK ? 56 : 0;

  if (color == P_BLACK)
    rights >>= 2;
  if (!(rights & 3))
    return;

  get_all_moves(board, color ^ COLOR_MASK, 1, opponent_moves);
  castle.src_square = (color == P_BLACK ? 60 : 4) | SPECIAL_MOVE;

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

void parse_castle(const Move *m, Castle *c) {
  assert(is_castle(m));

  c->king_start = m->src_square & 0x3F;
  c->rook_start = m->dest_square;
  c->dir = c->king_start > c->rook_start ? -1 : 1;

  c->king_end = c->king_start + (c->dir << 1);
  c->rook_end = c->king_start + c->dir;
}

void update_castling_rights(char start_square, ColoredPiece *rights) {
  switch (start_square) {
  case 0:
    *rights &= ~CASTLE_WQ;
    break;

  case 4:
    *rights &= ~CASTLE_W;
    break;

  case 7:
    *rights &= ~CASTLE_WK;
    break;

  case 56:
    *rights &= ~CASTLE_BQ;
    break;

  case 60:
    *rights &= ~CASTLE_B;
    break;

  case 63:
    *rights &= ~CASTLE_BK;
  }
}

char find_king(const ColoredPiece *board, PieceColor color) {
  char king_square;

  set_clear(&squares);
  find_pieces(board, KING | color, &squares);
  assert(squares.size == 1);
  king_square = squares.bytes[0];

  set_clear(&squares);
  return king_square;
}

char king_in_check(const ColoredPiece *board, PieceColor color) {
  Bitboard opponent_moves;
  get_all_moves(board, color ^ COLOR_MASK, 0, opponent_moves);
  return has_square(opponent_moves, find_king(board, color));
}

void filter_check(ColoredPiece *board, PieceColor color, Vector *moves) {
  Move *m;
  char i, target;
  Vector *new_moves = vector_get(&move_searches, 0);
  vector_clear(new_moves);

  for (i = 0; i < moves->size; i++) {
    m = vector_get(moves, i);

    // Castles are pre-filtered for check
    if (is_castle(m)) {
      vector_append(new_moves, m);
      continue;
    }

    target = board[m->dest_square];
    do_move(board, m, NONE, 1);

    if (!king_in_check(board, color))
      vector_append(new_moves, m);

    undo_move(board, m, target);
  }

  vector_swap(moves, new_moves);
}

void calc_stats(ColoredPiece *board, PieceColor color, PlayerStats *stats) {
  char player, king_square, rank, file, r, f, mask;
  const char *dir;
  Bitboard bit_control;
  ColoredPiece *piece, *last_piece = board + 64;

  stats->material = 0;
  for (piece = board; piece < last_piece; piece++)
    if (is_color(*piece, color))
      stats->material += MATERIAL_VALUES[*piece & PIECE_MASK];

  get_all_moves(board, color, 1, bit_control);

  stats->control = 0;
  for (rank = 0; rank < 8; rank++) {
    for (mask = 1; mask; mask <<= 1)
      if (bit_control[rank] & mask)
        stats->control++;

    if (rank > 1 && rank < 6)
      for (mask = 4; mask & 0x3C; mask <<= 1)
        if (bit_control[rank] & mask)
          stats->control++;
  }

  king_square = find_king(board, color);
  rank = square_rank(king_square);
  file = square_file(king_square);

  stats->safety = 0;
  if (king_in_check(board, color)) {
    stats->safety = -10;

    get_legal_moves(board, color, &temp_moves);
    if (vector_empty(&temp_moves)) {
      // Checkmate!
      stats->score = -1000;
      return;
    }
  }

  for (dir = DIRECTIONS; (dir - DIRECTIONS) < 16; dir += 2) {
    r = rank + dir[0];
    f = file + dir[1];
    if (!in_bounds(r, f) || is_color(board[(r << 3) + f], color))
      stats->safety += 2;
  }
  for (dir = PERIMETER; (dir - PERIMETER) < 32; dir += 2) {
    r = rank + dir[0];
    f = file + dir[1];
    if (!in_bounds(r, f) || is_color(board[(r << 3) + f], color))
      stats->safety++;
  }

  stats->score = (stats->material << 4) + stats->control + stats->safety;
}

Move *choose_move(ColoredPiece *board, PieceColor color, char search_depth) {
  int i, target1, target2, diff, max_diff = INT16_MIN;
  PlayerStats my_stats, their_stats;
  Move *move, *response, *max;
  PieceColor opponent = color ^ COLOR_MASK;
  Vector *moves = vector_get(&move_searches, search_depth);

  get_legal_moves(board, color, moves);
  if (vector_empty(moves))
    return NULL;
  if (moves->size == 1)
    return vector_get(moves, 0);

  for (i = 0; i < moves->size; i++) {
    move = vector_get(moves, i);
    target1 = board[move->dest_square];
    do_move(board, move, NONE, 1);

    if (search_depth > 1) {
      response = choose_move(board, opponent, search_depth - 1);
      if (response) {
        target2 = board[response->dest_square];
        do_move(board, response, NONE, 1);
      } else if (king_in_check(board, opponent)) {
        /* Checkmate */
        max = move;
        undo_move(board, move, target1);
        break;
      } else {
        /* Stalemate */
        undo_move(board, move, target1);
        continue;
      }
    }

    calc_stats(board, color, &my_stats);
    calc_stats(board, opponent, &their_stats);
    diff = my_stats.score - their_stats.score;
    if (diff > max_diff) {
      max_diff = diff;
      max = move;
    }

    if (search_depth > 1)
      undo_move(board, response, target2);

    undo_move(board, move, target1);
  }

  return max;
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

void get_slider_moves(const ColoredPiece *board, int src_square,
                      CaptureMode capture, int rank, int file, int dir_offset,
                      int dir_inc, Bitboard moves) {
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
char check_move(const ColoredPiece *board, int src_square, CaptureMode capture,
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
    throw("Invalid capture mode");
  }

  moves[rank] |= (1 << file);
  return 1;
}

char can_castle(const ColoredPiece *board, char row_offset,
                Bitboard opponent_moves, CastleSide side) {
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

  default:
    throw("Invalid castle side");
  }
}
