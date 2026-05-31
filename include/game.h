#ifndef GAME_H
#define GAME_H

#include "chess.h"

extern ColoredPiece board[8][8];

int init_game();
void cleanup_game();

#endif /* GAME_H */
