/** Game flow and UI logic. */

#include <assert.h>
#include <string.h>

#include "chess.h"
#include "game.h"
#include "draw.h"

DrawAdapter *draw = NULL;
ColoredPiece board[8][8];

int src_squares[10], *curr_sq;

/* Key handlers */
void select_piece(SDL_Keycode key);
void select_move(SDL_Keycode key);
void confirm(SDL_Keycode key);
void (*key_handler)(SDL_Keycode key) = select_piece;

/* Other private functions */
void show_controlled_squares();
void collect_moves();

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
    return draw->draw_screen();
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

void collect_moves()
{
    Bitboard moves;
    int square;

    while ((square = *curr_sq++))
    {
        clear_board(moves);
        get_moves(board, square, 0, moves);
    }
}

void select_piece(SDL_Keycode key)
{
    clear_board_highlights();
    char is_piece = strchr(PIECE_NAMES, (char)key) != NULL;
    curr_sq = src_squares;
    int i;

    if (key == 'b')
    {
        /* Select both bishops and pawns */
        find_pieces(board, WHITE_BISHOP, src_squares);
        collect_moves();
        find_file_pawns(board, P_WHITE, 1, curr_sq);
    }
    else if (key >= 'a' && key <= 'h')
        find_file_pawns(board, P_WHITE, key - 'a', src_squares);
    else if (is_piece)
        find_pieces(board, get_piece_named(key) & PIECE_MASK, src_squares);
    else if (key == SDLK_BACKSPACE)
    {
        for (i = 0; i < 64; i++)
            square_highlights[i] = RED;
        key_handler = confirm;
        draw->draw_board();
        return;
    }

    collect_moves();

    draw->draw_board();
}

void select_move(SDL_Keycode key)
{
    int rank, file, mask, *options;

    if (key >= 'a' && key <= 'h')
    {
        file = key - 'a';
    }
    else if (key > '0' && key < '9')
    {
        rank = key - '1';
    }
}

void confirm(SDL_Keycode key)
{
    if (key == SDLK_RETURN)
    {
        /* if () start_game(); */
    }

    key_handler = select_piece;
    clear_board_highlights();
    draw->draw_board();
}
