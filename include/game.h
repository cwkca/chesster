#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include "chess.h"

extern ColoredPiece board[64];

int init_game();
int start_game();
void handle_key(SDL_Keysym keysym);
void cleanup_game();

#endif /* GAME_H */
