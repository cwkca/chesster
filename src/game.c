/** Game flow logic. */

#include <assert.h>
#include <string.h>

#include "chess.h"
#include "game.h"
#include "draw.h"

DrawAdapter *draw = NULL;
ColoredPiece board[8][8];

/* Private function prototypes */
void show_controlled_squares();

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

    show_controlled_squares();
    return 0;
}

void handle_key(SDL_Keysym keysym)
{
}

void cleanup_game()
{
    if (draw)
        draw->cleanup();
}

/*
 * Private functions
 */

void show_controlled_squares()
{
    Bitboard white_control;
    Bitboard black_control;
    Bitboard dual_control;

    get_all_moves((ColoredPiece *)board, P_WHITE, 1, white_control);
    get_all_moves((ColoredPiece *)board, P_BLACK, 1, black_control);
    calc_board_overlap(white_control, black_control, dual_control);

    clear_board_highlights();
    highlight_squares(white_control, GREEN);
    highlight_squares(black_control, RED);
    highlight_squares(dual_control, YELLOW);

    draw->draw_board();
}
