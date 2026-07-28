/** Game-specific graphics. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL.h>

#include "chess.h"
#include "draw.h"
#include "draw_sdl.h"
#include "game.h"
#include "sdl_util.h"

const SDL_Rect CHESSBOARD_RECT = {80, 40, 480, 480};
const SDL_Color LIGHT_RGB = {200, 160, 130};
const SDL_Color DARK_RGB = {100, 60, 30};

#define LINE_SPACING 50
#define MARGIN 30
#define ERASE_WIDTH 80
#define ERASE_HEIGHT 150
#define STAT_TOLERANCE 5

int square_size;
SDL_Point material_disp[2], control_disp[2], safety_disp[2];
SDL_Rect stats_erase_rect[2];

/* Private function prototypes */
void draw_empty_board();
void draw_pieces();
void draw_piece(ColoredPiece piece, int file, int rank);
int draw_board_labels();
int layout_stats();
void calc_stat_colors(int stat0, int stat1, SDL_Color colors[2]);
int abs(int x);

int init_draw_sdl() {
  if (init_sdl())
    return 1;

  assert(CHESSBOARD_RECT.h == CHESSBOARD_RECT.w);
  assert(!(CHESSBOARD_RECT.h & 7)); /* Height should be a multiple of 8 */
  square_size = CHESSBOARD_RECT.h >> 3;

  return 0;
}

int draw_screen_sdl() {
  return draw_board_sdl() || draw_board_labels() || layout_stats();
}

int draw_board_sdl() {
  draw_empty_board();
  draw_pieces();
  return SDL_UpdateWindowSurface(window);
}

int update_stats_sdl() {
  int player;
  char number[10];
  SDL_Color material_colors[2], control_colors[2], safety_color;

  calc_stat_colors(curr_stats[0].material, curr_stats[1].material,
                   material_colors);
  calc_stat_colors(curr_stats[0].control, curr_stats[1].control,
                   control_colors);

  for (player = 0; player < 2; player++) {
    draw_rect(stats_erase_rect[player], 0, 0, 0);

    sprintf(number, "%d", curr_stats[player].material);
    draw_text(number, material_colors[player], material_disp[player],
              ALIGN_RIGHT);

    sprintf(number, "%d", curr_stats[player].control);
    draw_text(number, control_colors[player], control_disp[player],
              ALIGN_RIGHT);

    if (curr_stats[player].safety > 20)
      safety_color = GREEN;
    else if (curr_stats[player].safety > 10)
      safety_color = YELLOW;
    else
      safety_color = RED;

    sprintf(number, "%d", curr_stats[player].safety);
    draw_text(number, safety_color, safety_disp[player], ALIGN_RIGHT);
  }

  return SDL_UpdateWindowSurface(window);
}

void cleanup_draw_sdl() { cleanup_sdl(); }

/*
 * Private functions
 */

void draw_empty_board() {
  const Uint32 dark_color =
      SDL_MapRGB(winSurface->format, DARK_RGB.r, DARK_RGB.g, DARK_RGB.b);
  const Uint32 light_color =
      SDL_MapRGB(winSurface->format, LIGHT_RGB.r, LIGHT_RGB.g, LIGHT_RGB.b);

  SDL_Rect square_rect;
  square_rect.y = CHESSBOARD_RECT.y;
  square_rect.h = square_rect.w = square_size;

  int rank, file;
  for (rank = 7; rank >= 0; rank--) {
    square_rect.x = CHESSBOARD_RECT.x;
    for (file = 0; file < 8; file++) {
      SDL_Color board_color = (rank + file) % 2 ? LIGHT_RGB : DARK_RGB;
      SDL_Color highlight = square_highlights[(rank << 3) + file];

      Uint32 square_color;
      if (highlight.a) {
        square_color = SDL_MapRGB(winSurface->format,
                                  (highlight.r >> 1) + (board_color.r >> 1),
                                  (highlight.g >> 1) + (board_color.g >> 1),
                                  (highlight.b >> 1) + (board_color.b >> 1));
      } else
        square_color = SDL_MapRGB(winSurface->format, board_color.r,
                                  board_color.g, board_color.b);

      SDL_FillRect(winSurface, &square_rect, square_color);
      square_rect.x += square_size;
    }
    square_rect.y += square_size;
  }
}

void draw_pieces() {
  int rank, file;
  for (rank = 0; rank < 8; rank++)
    for (file = 0; file < 8; file++) {
      ColoredPiece piece = board[(rank << 3) + file];
      if (piece)
        draw_piece(piece, file, rank);
    }
}

void draw_piece(ColoredPiece piece, int file, int rank) {
  SDL_Rect pieceRect = {
      CHESSBOARD_RECT.x + file * square_size,
      CHESSBOARD_RECT.y + (7 - rank) * square_size,
      /* Width and height ignored */
      0,
      0,
  };

  SDL_BlitSurface(piece_imgs[piece], NULL, winSurface, &pieceRect);
}

int draw_board_labels() {
  char label[2] = {0};
  SDL_Point text_point;
  text_point.x = CHESSBOARD_RECT.x - (square_size >> 1);
  text_point.y = CHESSBOARD_RECT.y + (square_size >> 1);

  for (*label = '8'; *label > '0'; (*label)--) {
    draw_text(label, C_WHITE, text_point, ALIGN_CENTER);
    text_point.y += square_size;
  }

  text_point.x += square_size;
  for (*label = 'a'; *label <= 'h'; (*label)++) {
    draw_text(label, C_WHITE, text_point, ALIGN_CENTER);
    text_point.x += square_size;
  }

  return SDL_UpdateWindowSurface(window);
}

int layout_stats() {
  SDL_Point cursor;
  int left_edge = CHESSBOARD_RECT.x + CHESSBOARD_RECT.w;
  int center = (WINDOW_RECT.w + left_edge) >> 1;
  int left_margin = left_edge + MARGIN, right_margin = WINDOW_RECT.w - MARGIN;

  cursor.x = center;
  cursor.y = CHESSBOARD_RECT.y + (square_size >> 1);
  draw_text("Black", BROWN, cursor, ALIGN_CENTER);

  cursor.x = left_margin;
  cursor.y += LINE_SPACING;
  draw_text("Material", BLUE, cursor, ALIGN_LEFT);
  material_disp[1].y = cursor.y;

  cursor.y += LINE_SPACING;
  draw_text("Control", BLUE, cursor, ALIGN_LEFT);
  control_disp[1].y = cursor.y;

  cursor.y += LINE_SPACING;
  draw_text("King Safety", BLUE, cursor, ALIGN_LEFT);
  safety_disp[1].y = cursor.y;

  cursor.x = center;
  cursor.y = CHESSBOARD_RECT.y + ((CHESSBOARD_RECT.h + square_size) >> 1);
  draw_text("White", C_WHITE, cursor, ALIGN_CENTER);

  cursor.x = left_margin;
  cursor.y += LINE_SPACING;
  draw_text("Material", BLUE, cursor, ALIGN_LEFT);
  material_disp[0].y = cursor.y;

  cursor.y += LINE_SPACING;
  draw_text("Control", BLUE, cursor, ALIGN_LEFT);
  control_disp[0].y = cursor.y;

  cursor.y += LINE_SPACING;
  draw_text("King Safety", BLUE, cursor, ALIGN_LEFT);
  safety_disp[0].y = cursor.y;

  material_disp[0].x = control_disp[0].x = safety_disp[0].x =
      material_disp[1].x = control_disp[1].x = safety_disp[1].x = right_margin;

  stats_erase_rect[0].x = stats_erase_rect[1].x = WINDOW_RECT.w - ERASE_WIDTH;
  stats_erase_rect[1].y = CHESSBOARD_RECT.y + square_size;
  stats_erase_rect[0].y = stats_erase_rect[1].y + (CHESSBOARD_RECT.h >> 1);
  stats_erase_rect[0].w = stats_erase_rect[1].w = ERASE_WIDTH;
  stats_erase_rect[0].h = stats_erase_rect[1].h = ERASE_HEIGHT;

  return SDL_UpdateWindowSurface(window);
}

void calc_stat_colors(int stat0, int stat1, SDL_Color colors[2]) {
  int diff = stat1 - stat0;
  if (abs(diff) < STAT_TOLERANCE)
    colors[0] = colors[1] = YELLOW;
  else if (diff < 0) {
    colors[0] = GREEN;
    colors[1] = RED;
  } else {
    colors[0] = RED;
    colors[1] = GREEN;
  }
}

int abs(int x) { return x < 0 ? -x : x; }
