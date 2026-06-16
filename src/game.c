/** Game flow and UI logic. */

#include <assert.h>
#include <string.h>
#include <time.h> /* nanosleep */

#include "chess.h"
#include "game.h"
#include "draw.h"

const struct timespec move_delay = {0, 5e8};
DrawAdapter *draw = NULL;
ColoredPiece board[8][8];

Vector moves, new_moves;
ByteSet src_squares;
char restart;

/* Key handlers */
void select_piece(SDL_Keycode key);
void select_move(SDL_Keycode key);
void confirm(SDL_Keycode key);
void (*key_handler)(SDL_Keycode key) = select_piece;

/* Other private functions */
void show_controlled_squares();
void collect_moves();
void show_moves();
void do_move(ColoredPiece board[8][8], Move *move);
void move_black();

int init_game()
{
    draw = init_draw();
    if (!draw)
        return 1;

    init_piece_lookup();
    init_set(&src_squares, 10);
    init_vector(&moves, sizeof(Move), 20);
    init_vector(&new_moves, sizeof(Move), 20);

    return 0;
}

int start_game()
{
    assert(load_fen(STARTING_FEN) == 0);
    return draw->draw_screen();
}

void handle_key(SDL_Keysym keysym)
{
    if (key_handler)
        key_handler(keysym.sym);
}

void cleanup_game()
{
    cleanup_vector(&moves);
    cleanup_vector(&new_moves);
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
    Bitboard moves_board;
    char i, square;

    for (i = 0; i < src_squares.size; i++)
    {
        square = src_squares.bytes[i];
        clear_board(moves_board);
        get_moves(board, square, 0, moves_board);
        add_board_to_vector(square, moves_board, &moves);
    }
}

void show_moves()
{
    clear_board_highlights();

    char i;
    Move *m;
    for (i = 0; i < moves.size; i++)
    {
        m = vector_get(&moves, i);
        square_highlights[m->src_square] = BLUE;
        square_highlights[m->dest_square] = CYAN;
    }

    draw->draw_board();
}

void select_piece(SDL_Keycode key)
{
    char i;

    clear_set(&src_squares);
    clear_vector(&moves);
    char is_piece = strchr(PIECE_NAMES, (char)key) != NULL;
    restart = 0;

    if (key == 'b')
    {
        /* Select both bishops and pawns */
        find_pieces(board, WHITE_BISHOP, &src_squares);
        find_file_pawns(board, P_WHITE, 1, &src_squares);
    }
    else if (key >= 'a' && key <= 'h')
        find_file_pawns(board, P_WHITE, key - 'a', &src_squares);
    else if (is_piece)
        find_pieces(board, get_piece_named(key) & PIECE_MASK, &src_squares);
    else if (key == SDLK_BACKSPACE)
    {
        restart = 1;
        highlight_squares(FULL_BOARD, RED);
        key_handler = confirm;
        draw->draw_board();
        return;
    }

    collect_moves();
    if (moves.size > 0)
    {
        show_moves();
        key_handler = select_move;
    }
    else
    {
        clear_board_highlights();
        draw->draw_board();
        key_handler = select_piece;
    }
}

void select_move(SDL_Keycode key)
{
    char rank, file, i;
    Move *m;
    clear_vector(&new_moves);

    assert(src_squares.size > 0);
    char multi_piece = src_squares.size > 1;
    clear_set(&src_squares);

    if (key >= 'a' && key <= 'h')
    {
        file = key - 'a';
        for (i = 0; i < moves.size; i++)
        {
            m = vector_get(&moves, i);
            if (square_file(m->dest_square) == file ||
                (multi_piece && square_file(m->src_square) == file))
            {
                vector_append(&new_moves, m);
                set_add(&src_squares, m->src_square);
            }
        }
    }
    else if (key > '0' && key < '9')
    {
        rank = key - '1';
        for (i = 0; i < moves.size; i++)
        {
            m = vector_get(&moves, i);
            if (square_rank(m->dest_square) == rank ||
                (multi_piece && square_rank(m->src_square) == rank))
            {
                vector_append(&new_moves, m);
                set_add(&src_squares, m->src_square);
            }
        }
    }
    else if (key == SDLK_RETURN && moves.size == 1)
        confirm(key);

    swap_vectors(&moves, &new_moves);
    show_moves();

    if (moves.size == 0)
        select_piece(key);
    else if (moves.size == 1)
        key_handler = confirm;
}

void confirm(SDL_Keycode key)
{
    if (key == SDLK_RETURN)
    {
        if (restart)
            start_game();
        else
        {
            assert(moves.size == 1);
            do_move(board, vector_get(&moves, 0));
            move_black();
        }
    }

    key_handler = select_piece;
    clear_board_highlights();
    draw->draw_board();
}

void do_move(ColoredPiece board[8][8], Move *move)
{
    ColoredPiece *squares = (ColoredPiece *)board;
    ColoredPiece piece = squares[move->src_square];
    squares[move->src_square] = NONE;
    squares[move->dest_square] = piece;
}

void move_black()
{
    clear_board_highlights();
    draw->draw_board();
    nanosleep(&move_delay, NULL);

    clear_vector(&src_squares);
    find_all_pieces(board, P_BLACK, &src_squares);
    assert(src_squares.size);

    clear_vector(&moves);
    collect_moves();
    if (moves.size)
        do_move(board, choose_random_elt(&moves));
    else
    {
        printf("You win!\n");
        highlight_squares(FULL_BOARD, GREEN);
        draw->draw_board();
        key_handler = NULL;
    }
}
