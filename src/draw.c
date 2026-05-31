/** Game-specific graphics. */

#include <stdlib.h>

#include "draw.h"
#include "draw_sdl.h"
#include "draw_term.h"
#include "chess.h"

DrawAdapter TERM_DRAW = {
    &draw_board_term,
    &draw_pieces_term,
    &cleanup_term,
};

DrawAdapter SDL_DRAW = {
    &draw_board_sdl,
    &draw_pieces_sdl,
    &cleanup_sdl,
};

DrawAdapter *init_draw()
{
    return !getenv("CHESSTERM") && init_draw_sdl() == 0
               ? &SDL_DRAW
               : &TERM_DRAW;
}
