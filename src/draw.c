/** Game-specific graphics. */

#include <stdlib.h>
#include <SDL.h>

#include "draw.h"
#include "draw_sdl.h"
#include "draw_term.h"

const SDL_Color C_BLACK = {0, 0, 0, 0xFF};
const SDL_Color C_WHITE = {0xFF, 0xFF, 0xFF, 0xFF};
const SDL_Color GREY = {0x7F, 0x7F, 0x7F, 0xFF};
const SDL_Color CLEAR = {0};
const SDL_Color RED = {0xFF, 0, 0, 0xFF};
const SDL_Color YELLOW = {0xFF, 0xFF, 0, 0xFF};
const SDL_Color GREEN = {0, 0xFF, 0, 0xFF};
const SDL_Color CYAN = {0, 0xFF, 0xFF, 0xFF};
const SDL_Color BLUE = {0, 0, 0xFF, 0xFF};
const SDL_Color MAGENTA = {0xFF, 0, 0xFF, 0xFF};

SDL_Color square_highlights[64] = {0};

DrawAdapter TERM_DRAW = {
    &draw_screen_term,
    &draw_board_term,
    &cleanup_term,
};

DrawAdapter SDL_DRAW = {
    &draw_screen_sdl,
    &draw_board_sdl,
    &cleanup_sdl,
};

DrawAdapter *init_draw()
{
    return !getenv("CHESSTERM") && init_draw_sdl() == 0
               ? &SDL_DRAW
               : &TERM_DRAW;
}

void highlight_board(SDL_Color (*highlight)(int, int))
{
    int rank, file;
    SDL_Color *square = square_highlights;
    for (rank = 0; rank < 8; rank++)
        for (file = 0; file < 8; file++)
            *square++ = highlight(rank, file);
}
