/** Game flow logic. */

#include <assert.h>
#include <stddef.h>

#include "game.h"
#include "draw.h"

DrawAdapter *draw = NULL;
ColoredPiece board[8][8];

const SDL_Color HIGHLIGHT_COLOR = {0, 0, 255, 255};

int highlight;

/* Private function prototypes */
int highlight_rank(int rank, int file);
int highlight_file(int rank, int file);
int no_highlight(int rank, int file);

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

void handle_key(SDL_Keycode key)
{
    if (key > '0' && key < '9')
    {
        highlight = key - '1';
        highlight_board(draw, HIGHLIGHT_COLOR, highlight_rank);
    }
    else if (key >= 'a' && key <= 'h')
    {
        highlight = key - 'a';
        highlight_board(draw, HIGHLIGHT_COLOR, highlight_file);
    }

    else
        highlight_board(draw, HIGHLIGHT_COLOR, no_highlight);
}

int highlight_rank(int rank, int file)
{
    return rank == highlight;
}

int highlight_file(int rank, int file)
{
    return file == highlight;
}

int no_highlight(int rank, int file)
{
    return 0;
}

void cleanup_game()
{
    if (draw)
        draw->cleanup();
}
