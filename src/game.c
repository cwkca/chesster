/** Game flow logic. */

#include <assert.h>
#include <string.h>

#include "chess.h"
#include "game.h"
#include "draw.h"

DrawAdapter *draw = NULL;
ColoredPiece board[8][8];

int src_squares[10], dest_squares[10];
Bitboard moves;

/* Key handlers */
void select_piece(SDL_Keycode key);
void select_file(SDL_Keycode key);
void select_rank(SDL_Keycode key);
void select_rank_or_file(SDL_Keycode key);
void confirm(SDL_Keycode key);
void (*key_handler)(SDL_Keycode key) = select_piece;

/* Other private functions */
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
    key_handler(keysym.sym);
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

    get_all_moves(board, P_WHITE, 1, white_control);
    get_all_moves(board, P_BLACK, 1, black_control);
    calc_board_overlap(white_control, black_control, dual_control);

    clear_board_highlights();
    highlight_squares(white_control, GREEN);
    highlight_squares(black_control, RED);
    highlight_squares(dual_control, YELLOW);

    draw->draw_board();
}

void select_piece(SDL_Keycode key)
{
    clear_board(moves);
    *src_squares = *dest_squares = -1;
    int *square = src_squares;

    clear_board_highlights();
    char is_piece = strchr(PIECE_NAMES, (char)key) != NULL;

    if (key == 'b')
    {
        /* Select both bishops and pawns */
        find_pieces(board, WHITE_BISHOP, src_squares);
        while (*square >= 0)
            get_moves(board, *square++, 0, moves);
        find_file_pawns(board, P_WHITE, 1, square);
    }
    else if (key >= 'a' && key <= 'h')
        find_file_pawns(board, P_WHITE, key - 'a', src_squares);
    else if (is_piece)
        find_pieces(board, get_piece_named(key) & PIECE_MASK, src_squares);

    while (*square >= 0)
        get_moves(board, *square++, 0, moves);

    key_handler = board_empty(moves) ? select_piece : select_rank_or_file;

    for (square = src_squares; *square >= 0; square++)
        square_highlights[*square] = BLUE;
    highlight_squares(moves, CYAN);

    draw->draw_board();
}

void select_rank_or_file(SDL_Keycode key)
{
    int square_count;

    if (key >= 'a' && key <= 'h')
    {
        square_count = find_file_squares(moves, key - 'a', dest_squares);

        if (square_count == 1)
        {
            key_handler = confirm;
            return;
        }
        else if (square_count > 1)
        {
            /* Todo */
            return;
        }
    }
    else if (key > '0' && key < '9')
    {
        square_count = find_rank_squares(moves, key - '1', dest_squares);

        if (square_count == 1)
        {
            key_handler = confirm;
            return;
        }
        else if (square_count > 1)
        {
            /* Todo */
            return;
        }
    }

    select_piece(key);
}

void confirm(SDL_Keycode key)
{
    if (key == SDLK_ESCAPE)
    {
        *src_squares = -1;
        key_handler = select_piece;
        clear_board_highlights();
        draw->draw_board();
        return;
    }

    /* Todo */
}
