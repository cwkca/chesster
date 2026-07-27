#ifndef DRAW_H
#define DRAW_H

#include "chess.h"
#include <SDL.h>

extern const SDL_Color C_BLACK, C_WHITE, GREY, CLEAR;
extern const SDL_Color RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA;

extern SDL_Color square_highlights[64];

typedef struct {
  int (*draw_screen)();
  int (*draw_board)();
  int (*draw_stats)();
  void (*cleanup)();
} DrawAdapter;

DrawAdapter *init_draw();

void highlight_squares(const Bitboard squares, SDL_Color color);
void clear_board_highlights();

#endif /* DRAW_H */
