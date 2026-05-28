/** Game logic. */

#include "game.h"
#include "draw.h"

const char *pieces = "kqrbnp";

int init_screen()
{ 
    if (init_draw()) return 1;

    /* Hack */
    char *board = "rnbqkbnrpppppppp................................PPPPPPPPRNBQKBNR";

    draw_sdl_chessboard();
    if (draw_pieces(board))
        return 1;
    return 0;
}
