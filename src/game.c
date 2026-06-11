/** Game flow logic. */

#include <assert.h>
#include <string.h>

#include "chess.h"
#include "game.h"
#include "draw.h"

DrawAdapter *draw = NULL;
ColoredPiece board[8][8];

int src_squares[10];
int *curr_sq, src_sq, dest_sq;
Bitboard all_moves;
int rank_srcs[8][10], file_srcs[8][10], *r_src[8], *f_src[8];

/* Key handlers */
void select_piece(SDL_Keycode key);
void select_file(SDL_Keycode key);
void select_rank(SDL_Keycode key);
void select_rank_or_file(SDL_Keycode key);
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

void collect_moves()
{
    Bitboard moves;
    int rank, file, square, mask;

    while (*curr_sq >= 0)
    {
        clear_board(moves);
        square = *curr_sq++;
        get_moves(board, square, 0, moves);
        for (rank = 0; rank < 8; rank++)
            if (moves[rank])
            {
                *r_src[rank]++ = square;
                all_moves[rank] = all_moves[rank] | moves[rank];

                mask = 1;
                for (file = 0; file < 8; file++, mask <<= 1)
                    if (moves[rank] & mask)
                        *f_src[file]++ = square;
            }
    }
}

void select_piece(SDL_Keycode key)
{
    clear_board(all_moves);
    *src_squares = -1;
    curr_sq = src_squares;
    src_sq = dest_sq = -1;

    clear_board_highlights();
    char is_piece = strchr(PIECE_NAMES, (char)key) != NULL;

    int i;
    for (i = 0; i < 8; i++)
    {
        r_src[i] = rank_srcs[i];
        f_src[i] = file_srcs[i];
    }

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
    for (i = 0; i < 8; i++)
    {
        *r_src[i] = -1;
        *f_src[i] = -1;
    }

    key_handler = board_empty(all_moves) ? select_piece : select_rank_or_file;

    for (curr_sq = src_squares; *curr_sq >= 0; curr_sq++)
        square_highlights[*curr_sq] = BLUE;
    highlight_squares(all_moves, CYAN);

    draw->draw_board();
}

void select_rank_or_file(SDL_Keycode key)
{
    int rank, file, mask, *options;

    if (key >= 'a' && key <= 'h')
    {
        file = key - 'a';
        options = file_srcs[file];
        if (*options >= 0 && options[1] == -1)
        {
            src_sq = *options;
            mask = 1 << file;
            for (rank = 0; rank < 8; rank++)
                if (all_moves[rank] & mask)
                {
                    dest_sq = (rank << 3) + file;
                    break;
                }
        }
    }
    else if (key > '0' && key < '9')
    {
        rank = key - '1';
        options = rank_srcs[rank];
        if (*options >= 0 && options[1] == -1)
        {
            src_sq = *options;
            mask = 1;
            for (file = 0; file < 8; file++, mask <<= 1)
                if (all_moves[rank] & mask)
                {
                    dest_sq = (rank << 3) + file;
                    break;
                }
        }
    }

    if (src_sq >= 0)
    {
        clear_board_highlights();
        square_highlights[src_sq] = BLUE;
        square_highlights[dest_sq] = CYAN;
        draw->draw_board();
        key_handler = confirm;
    }
    else
        select_piece(key);
}

void confirm(SDL_Keycode key)
{
    if (key == SDLK_RETURN)
    {
        if (src_sq < 0)
        {
            start_game();
            return;
        }
        else
        {
            ColoredPiece *squares = (ColoredPiece *)board;
            ColoredPiece piece = squares[src_sq];
            squares[src_sq] = 0;
            squares[dest_sq] = piece;
        }
    }

    key_handler = select_piece;
    clear_board_highlights();
    draw->draw_board();
}
