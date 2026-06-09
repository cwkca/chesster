/** Game flow logic. */

#include <assert.h>
#include <stddef.h>

#include "game.h"
#include "draw.h"

DrawAdapter *draw = NULL;
ColoredPiece board[8][8];

const SDL_Color NO_COLOR = {0};
const SDL_Color HIGHLIGHT_COLOR = {0, 0, 255, 255};

int highlight;

/* Private function prototypes */
SDL_Color highlight_rank(int rank, int file);
SDL_Color highlight_file(int rank, int file);
SDL_Color no_highlight(int rank, int file);

int init_game()
{
    draw = init_draw();
    if (!draw)
        return 1;

    init_piece_lookup();
    return 0;
}

int start_game()
{
    assert(load_fen(STARTING_FEN) == 0);
    if (draw->draw_screen())
        return 1;
    return 0;
}

void handle_key(SDL_Keysym keysym)
{
    SDL_Keycode key = keysym.sym;
    if (key > SDLK_SCANCODE_MASK)
        return;

    if (key > '0' && key < '9')
    {
        highlight = key - '1';
        highlight_board(highlight_rank);
    }
    else if (key >= 'a' && key <= 'h')
    {
        highlight = key - 'a';
        highlight_board(highlight_file);
    }
    else
        highlight_board(no_highlight);

    draw->draw_board();
}

void cleanup_game()
{
    if (draw)
        draw->cleanup();
}

/*
 * Private functions
 */

SDL_Color highlight_rank(int rank, int file)
{
    return rank == highlight ? HIGHLIGHT_COLOR : NO_COLOR;
}

SDL_Color highlight_file(int rank, int file)
{
    return file == highlight ? HIGHLIGHT_COLOR : NO_COLOR;
}

SDL_Color no_highlight(int rank, int file)
{
    return NO_COLOR;
}
