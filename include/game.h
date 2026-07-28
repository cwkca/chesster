#ifndef GAME_H
#define GAME_H

#include "chess.h"
#include <SDL.h>

extern ColoredPiece board[65];
extern PlayerStats curr_stats[2];

int init_game();
int start_game();
void handle_key(SDL_Keysym keysym);
void cleanup_game();

#endif /* GAME_H */
