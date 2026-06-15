/* Data structures. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"

#define INITIAL_VECTOR_MAGN 4
#define VECTOR_INC_MAGN 3

const Bitboard FULL_BOARD = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void clear_board(Bitboard board)
{
    memset(board, 0, 8);
}

char board_empty(const Bitboard board)
{
    int rank;
    for (rank = 0; rank < 8; rank++)
        if (board[rank])
            return 0;
    return 1;
}

void set_square(int square, Bitboard board)
{
    board[square_rank(square)] |= 1 << square_file(square);
}

void calc_board_overlap(const Bitboard a, const Bitboard b, Bitboard overlap)
{
    int rank;
    for (rank = 0; rank < 8; rank++)
        overlap[rank] = a[rank] & b[rank];
}

void init_vector(MoveVector *v)
{
    assert(v);

    v->memsize = sizeof(Move) << INITIAL_VECTOR_MAGN;
    v->moves = malloc(v->memsize);
    if (!v->moves)
    {
        printf("Unable to allocate %d bytes of memory\n", v->memsize);
        exit(1);
    }

    v->size = 0;
    v->capacity = 1 << INITIAL_VECTOR_MAGN;
}

Move *vector_get(MoveVector *v, char i)
{
    assert(v && v->moves);
    if (i < 0 || i >= v->size)
    {
        printf("Vector index %d out of bounds (size %d)\n", i, v->size);
        exit(1);
    }

    return v->moves + i;
}

void add_to_vector(MoveVector *v, Move move)
{
    assert(v && v->size <= v->capacity);

    if (v->size == v->capacity)
    {
        int new_memsize = v->memsize + (sizeof(Move) << VECTOR_INC_MAGN);
        Move *new_moves = realloc(v->moves, new_memsize);
        if (!new_moves)
        {
            printf("Unable to allocate %d bytes of memory\n", new_memsize);
            exit(1);
        }

        v->moves = new_moves;
        v->memsize = new_memsize;

        int new_capacity = v->capacity + (1 << VECTOR_INC_MAGN);
        printf("Increasing vector capacity from %d to %d\n", v->capacity, new_capacity);
        v->capacity = new_capacity;
    }

    v->moves[v->size++] = move;
}

void swap_vectors(MoveVector *a, MoveVector *b)
{
    MoveVector temp = *a;
    *a = *b;
    *b = temp;
}

void cleanup_vector(MoveVector *v)
{
    if (!v->moves)
        return;
    free(v->moves);
    v->size = v->capacity = v->memsize = 0;
}
