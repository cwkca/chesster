/** Game logic. */

#include "game.h"
#include "draw.h"

const char *pieces = "kqrbnp";
char *board = "rnbqkbnrpppppppp................................PPPPPPPPRNBQKBNR";

int init_screen()
{ 
    if (init_draw()) return 1;
    
    draw_chessboard();
    if (draw_pieces(board))
        return 1;
    return 0;
}
