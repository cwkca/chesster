/** Game flow logic. */

#include <assert.h>
#include <stddef.h>

#include "game.h"
#include "draw.h"

DrawAdapter *draw = NULL;
ColoredPiece board[8][8];

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
    if (isprint(key))
        printf("%c\n", (char)key);
}

void cleanup_game()
{
    if (draw)
        draw->cleanup();
}
