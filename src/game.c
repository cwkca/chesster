/** Game flow and UI logic. */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> /* nanosleep, time */

#include "chess.h"
#include "draw.h"
#include "game.h"

const struct timespec move_delay = {0, 2e8};
DrawAdapter *draw = NULL;
ColoredPiece board[65];
const int BOARD_BYTES = sizeof(board);
PlayerStats curr_stats[2];

Vector moves, new_moves, boards;
ByteSet src_squares;

#define MOVE_LEN 10
#define RESTART_KEY SDLK_BACKSPACE
#define SHOW_CTRL SDLK_SPACE
char move_str[MOVE_LEN], *move_end;
char move, filter_rank, filter_file, filter_castle, dest_square;

#define MAX_SEARCH_DEPTH 5
#define DEFAULT_SEARCH_DEPTH 3
int search_depth = DEFAULT_SEARCH_DEPTH;

/* Key handlers */
void select_piece(SDL_Keycode key);
void select_move(SDL_Keycode key);
void (*key_handler)(SDL_Keycode key) = select_piece;

/* Move filtering */
void filter_moves(char (*filter)(Move *));
char by_capture(Move *m);
char by_rank(Move *m);
char by_file(Move *m);
char by_dest(Move *m);
char by_castle(Move *m);
char get_dest_square();

/* Other private functions */
int update_stats();
void show_controlled_squares();
void show_moves();
void do_move(ColoredPiece *board, Move *move, Piece promote, char testing);
char move_black();

int init_game() {
  srand(time(NULL));

  draw = init_draw();
  if (!draw)
    return 1;

  init_chess(MAX_SEARCH_DEPTH + 1);
  set_init(&src_squares, 16);
  vector_init(&moves, sizeof(Move), MOVE_VECTOR_SIZE);
  vector_init(&new_moves, sizeof(Move), MOVE_VECTOR_SIZE);
  vector_init(&boards, BOARD_BYTES, 50);

  char *depth_str = getenv("CHESSTER_DEPTH");
  if (depth_str) {
    search_depth = strtol(depth_str, NULL, 10);
    if (search_depth > MAX_SEARCH_DEPTH || search_depth < 1) {
      printf("Invalid search depth %d\n", search_depth);
      search_depth = DEFAULT_SEARCH_DEPTH;
    }
  }

  return 0;
}

int start_game() {
  vector_clear(&boards);
  move = 0;

  assert(load_fen(STARTING_FEN) == 0);
  vector_append(&boards, board);
  board[64] = CASTLE_ALL;

  *move_str = 0;
  move_end = move_str;

  return draw->draw_screen() || update_stats();
}

void handle_key(SDL_Keysym keysym) {
  if (key_handler)
    key_handler(keysym.sym);
}

void cleanup_game() {
  cleanup_chess();

  set_cleanup(&src_squares);
  vector_cleanup(&moves);
  vector_cleanup(&new_moves);
  vector_cleanup(&boards);

  if (draw)
    draw->cleanup();
}

/*
 * Private functions
 */

void select_piece(SDL_Keycode key) {
  char i;
  char is_piece = strchr(PIECE_NAMES, (char)key) != NULL;
  set_clear(&src_squares);
  vector_clear(&moves);

  assert(key_handler == select_piece);
  move_end = move_str;
  *(move_end++) = key;

  if (key == SDLK_LEFT && move > 0) {
    memcpy(board, vector_get(&boards, --move), BOARD_BYTES);
    update_stats();
  } else if (key == SDLK_RIGHT && move < boards.size - 1) {
    memcpy(board, vector_get(&boards, ++move), BOARD_BYTES);
    update_stats();
  } else if (key == SHOW_CTRL) {
    show_controlled_squares();
    return;
  } else if (key == RESTART_KEY) {
    highlight_squares(FULL_BOARD, RED);
    draw->draw_board();
    key_handler = select_move;
    return;
  } else if (key == 'b') {
    /* Select both bishops and pawns */
    find_pieces(board, WHITE_BISHOP, &src_squares);
    find_file_pawns(board, P_WHITE, 1, &src_squares);
  } else if (is_file(key))
    find_file_pawns(board, P_WHITE, key - 'a', &src_squares);
  else if (is_piece)
    find_pieces(board, get_piece_named(key) & PIECE_MASK, &src_squares);
  else if (key == 'o')
    get_castles(board, P_WHITE, &moves);

  get_moves_from(board, &src_squares, &moves);
  filter_check(board, P_WHITE, &moves);
  set_clear(&src_squares);

  if (moves.size > 0) {
    show_moves();
    key_handler = select_move;
  } else {
    clear_board_highlights();
    draw->draw_board();
  }
}

void select_move(SDL_Keycode key) {
  char complete;

  if (key == SDLK_RETURN) {
    if (*move_str == RESTART_KEY)
      start_game();
    else if (moves.size == 1) {
      do_move(board, vector_get(&moves, 0), QUEEN, 0);
      complete = !move_black();
      update_stats();

      boards.size = ++move;
      vector_append(&boards, board);

      if (complete) {
        if (king_in_check(board, P_BLACK)) {
          printf("You win!\n");
          highlight_squares(FULL_BOARD, GREEN);
        } else {
          printf("Stalemate!\n");
          highlight_squares(FULL_BOARD, GREY);
        }
        draw->draw_board();
        key_handler = NULL;
        return;
      }
    }
  }

  vector_clear(&new_moves);

  if (move_end - move_str < MOVE_LEN && is_move_char(key))
    *(move_end++) = key;

  if (key == 'x')
    filter_moves(by_capture);
  else if (is_file(key)) {
    filter_file = key - 'a';
    filter_moves(by_file);
  } else if (is_rank(key)) {
    filter_rank = key - '1';
    dest_square = get_dest_square();

    if (dest_square >= 0)
      filter_moves(by_dest);
    else
      filter_moves(by_rank);
  } else if (key == 'o' && *move_str == 'o') {
    *move_end = 0;
    filter_castle =
        strspn(move_str, "o") > 2 ? CASTLE_QUEENSIDE : CASTLE_KINGSIDE;

    vector_clear(&moves);
    get_castles(board, P_WHITE, &moves);
    filter_moves(by_castle);
  }

  vector_swap(&moves, &new_moves);

  if (!moves.size)
    key_handler = select_piece;

  show_moves();
}

void filter_moves(char (*filter)(Move *)) {
  char i;
  Move *m;

  for (i = 0; i < moves.size; i++) {
    m = vector_get(&moves, i);
    if (filter(m))
      vector_append(&new_moves, m);
  }
}

char by_capture(Move *m) { return board[m->dest_square]; }

char by_rank(Move *m) {
  return square_rank(m->src_square) == filter_rank ||
         square_rank(m->dest_square) == filter_rank;
}

char by_file(Move *m) {
  return square_file(m->src_square) == filter_file ||
         square_file(m->dest_square) == filter_file;
}

char by_dest(Move *m) { return m->dest_square == dest_square; }

char by_castle(Move *m) {
  return square_file(m->dest_square) ==
         (filter_castle == CASTLE_QUEENSIDE ? 0 : 7);
}

char get_dest_square() {
  char *dest_search = move_end - 2;
  if (dest_search < move_str)
    return -1;

  if (is_file(dest_search[0]) && is_rank(dest_search[1]))
    return ((dest_search[1] - '1') << 3) + (dest_search[0] - 'a');

  return -1;
}

int update_stats() {
  calc_stats(board, P_WHITE, curr_stats);
  calc_stats(board, P_BLACK, curr_stats + 1);
  return draw->update_stats();
}

void show_controlled_squares() {
  Bitboard white_control;
  Bitboard black_control;
  Bitboard dual_control;

  get_all_moves(board, P_WHITE, 1, white_control);
  get_all_moves(board, P_BLACK, 1, black_control);
  calc_board_overlap(white_control, black_control, dual_control);

  clear_board_highlights();
  highlight_squares(white_control, GREEN);
  highlight_squares(black_control, RED);
  highlight_squares(dual_control, YELLOW);

  draw->draw_board();
}

void show_moves() {
  char i;
  Move *m;
  Castle c;

  clear_board_highlights();
  for (i = 0; i < moves.size; i++) {
    m = vector_get(&moves, i);
    if (is_castle(m)) {
      parse_castle(m, &c);
      square_highlights[c.king_start] = square_highlights[c.rook_start] = BLUE;
      square_highlights[c.king_end] = square_highlights[c.rook_end] = CYAN;
    } else {
      square_highlights[m->src_square] = BLUE;
      square_highlights[m->dest_square] = CYAN;
    }
  }

  draw->draw_board();
}

char move_black() {
  Move *m;

  clear_board_highlights();
  draw->draw_board();
  nanosleep(&move_delay, NULL);

  m = choose_move(board, P_BLACK, search_depth);
  if (m)
    do_move(board, m, QUEEN, 0);

  return m != NULL;
}
