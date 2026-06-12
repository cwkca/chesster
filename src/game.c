/** Game flow logic. */

#include <assert.h>
#include <string.h>

#include "chess.h"
#include "game.h"
#include "draw.h"

const Bitboard FULL_BOARD = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
DrawAdapter *draw = NULL;
ColoredPiece board[8][8];

char src_squares[10], new_srcs[10], dest_squares[64];
char src_sq, dest_sq, sqi;
Bitboard src_boards[10];

/* Key handlers */
void select_piece(SDL_Keycode key);
void select_file(SDL_Keycode key);
void select_rank(SDL_Keycode key);
void select_move(SDL_Keycode key);
void confirm(SDL_Keycode key);
void (*key_handler)(SDL_Keycode key) = select_piece;

/* Other private functions */
void show_controlled_squares();
int collect_moves();
void show_moves();

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

int collect_moves()
{
    int rank, file, mask, src_square, dest_square, move_count = 0;
    Bitboard *moves;

    for (; src_squares[sqi] >= 0; sqi++)
    {
        src_square = src_squares[sqi];
        moves = src_boards + sqi;
        clear_board(*moves);
        get_moves(board, src_square, 0, *moves);

        for (rank = 0; rank < 8; rank++)
            if ((*moves)[rank])
            {
                mask = 1;
                for (file = 0; file < 8; file++, mask <<= 1)
                    if ((*moves)[rank] & mask)
                    {
                        move_count++;
                        dest_square = (rank << 3) + file;

                        if (dest_squares[dest_square] >= 0)
                            /* Square already mapped. Mark as multiple. */
                            dest_squares[dest_square] = 64;
                        else
                            dest_squares[dest_square] = src_square;
                    }
            }
    }

    return move_count;
}

void show_moves()
{
    clear_board_highlights();
    if (src_sq >= 0)
    {
        clear_board_highlights();
        square_highlights[src_sq] = BLUE;
        square_highlights[dest_sq] = CYAN;
    }
    else
    {
        for (sqi = 0; src_squares[sqi] >= 0; sqi++)
            square_highlights[src_squares[sqi]] = BLUE;
        for (sqi = 0; sqi < 64; sqi++)
            if (dest_squares[sqi] >= 0)
                square_highlights[sqi] = CYAN;
    }
    draw->draw_board();
}

void select_piece(SDL_Keycode key)
{
    *src_squares = -1;
    memset(dest_squares, -1, sizeof(dest_squares));
    src_sq = dest_sq = -1;
    sqi = 0;
    int move_count = 0;

    clear_board_highlights();
    char is_piece = strchr(PIECE_NAMES, (char)key) != NULL;

    if (key == 'b')
    {
        /* Select both bishops and pawns */
        find_pieces(board, WHITE_BISHOP, src_squares);
        move_count += collect_moves();
        find_file_pawns(board, P_WHITE, 1, src_squares + sqi);
    }
    else if (key >= 'a' && key <= 'h')
        find_file_pawns(board, P_WHITE, key - 'a', src_squares);
    else if (is_piece)
        find_pieces(board, get_piece_named(key) & PIECE_MASK, src_squares);
    else if (key == SDLK_BACKSPACE)
    {
        highlight_squares(FULL_BOARD, RED);
        key_handler = confirm;
        draw->draw_board();
        return;
    }

    move_count += collect_moves();
    key_handler = move_count ? select_move : select_piece;

    show_moves();
}

void select_move(SDL_Keycode key)
{
    char rank, file, curr_sq, src_i, new_i, r, sq, mask;
    char src_square = -1;

    if (key >= 'a' && key <= 'h')
    {
        file = key - 'a';

        for (curr_sq = file; curr_sq < 64; curr_sq += 8)
        {
            if (dest_squares[curr_sq] < 0)
                continue;
            if (dest_squares[curr_sq] > 63)
            {
                printf("Ambiguous\n");
                break;
            }
            if (src_square < 0)
            {
                src_square = dest_squares[curr_sq];
                dest_sq = curr_sq;
                continue;
            }

            /* Narrowing down */
            src_square = 64;
            dest_sq = -1;
            break;
        }

        if (src_square < 64)
        {
            printf("Searching sources\n");
            /* Search sources in this file */
            for (sqi = 0; src_squares[sqi] >= 0; sqi++)
                if ((src_squares[sqi] & 7) == file)
                {
                    if (src_square < 0)
                        src_square = src_squares[sqi];
                    else
                    {
                        /* Found multiple */
                        src_square = -1;
                        break;
                    }
                }
        }
        else
        {
            /* Eliminate squares in other files */
            printf("Eliminate squares in other files\n");
            for (sq = 0; sq < 64; sq++)
                if ((sq & 7) != file && (dest_squares[sq] & 7) != file)
                    dest_squares[sq] = -1;

            /* Update sources */
            printf("Update sources\n");
            new_i = 0;
            mask = 1 << file;
            for (src_i = 0; src_squares[src_i] >= 0; src_i++)
                /* Keep sources in this file */
                if ((src_squares[src_i] & 7) == file)
                    new_srcs[new_i++] = src_squares[src_i];
                else
                    /* Keep sources with destinations in this file */
                    for (r = 0; r < 8; r++)
                        if (src_boards[src_i][r] & mask)
                            new_srcs[new_i++] = src_squares[src_i];

            printf("Storing result\n");
            for (src_i = 0; src_i < new_i; src_i++)
                src_squares[src_i] = new_srcs[src_i];
            src_squares[src_i] = -1;
        }

        printf("Checking result\n");
        if (src_square >= 0)
            src_sq = src_square;
    }
    else if (key > '0' && key < '9')
    {
        rank = key - '1';
    }

    if (src_sq >= 0 && dest_sq >= 0)
    {
        printf("Showing result\n");
        clear_board_highlights();
        square_highlights[src_sq] = BLUE;
        square_highlights[dest_sq] = CYAN;
        key_handler = confirm;
    }
    else if (src_square < 64)
    {
        printf("Nothing found\n");
        select_piece(key);
    }
    else
    {
        printf("Showing options\n");
        show_moves();
    }
}

void confirm(SDL_Keycode key)
{
    if (key == SDLK_RETURN)
    {
        if (src_sq < 0)
            start_game();
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
