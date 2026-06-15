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

void init_vector(Vector *v, char elt_size)
{
    assert(v);
    assert(elt_size >= 0);

    if (elt_size == 1)
        v->elt_magn = 0;
    else if (elt_size == 2)
        v->elt_magn = 1;
    else if (elt_size < 5)
        v->elt_magn = 2;
    else
    {
        printf("Unsupported vector element size %d\n", elt_size);
        exit(1);
    }

    v->memsize = 1 << (v->elt_magn + INITIAL_VECTOR_MAGN);
    v->data = malloc(v->memsize);
    if (!v->data)
    {
        printf("Unable to allocate %d bytes of memory\n", v->memsize);
        exit(1);
    }

    v->size = 0;
    v->capacity = 1 << INITIAL_VECTOR_MAGN;
}

void *vector_get(Vector *v, char i)
{
    assert(v);
    if (i < 0 || i >= v->size)
    {
        printf("Vector index %d out of bounds (size %d)\n", i, v->size);
        exit(1);
    }

    return v->data + (i << v->elt_magn);
}

void add_to_vector(Vector *v, void *elt)
{
    assert(v && v->size <= v->capacity);

    if (v->size == v->capacity)
    {
        int new_memsize = v->memsize + (1 << (v->elt_magn + VECTOR_INC_MAGN));
        void *new_data = realloc(v->data, new_memsize);
        if (!new_data)
        {
            printf("Unable to allocate %d bytes of memory\n", new_memsize);
            exit(1);
        }

        v->data = new_data;
        v->memsize = new_memsize;

        int new_capacity = v->capacity + (1 << VECTOR_INC_MAGN);
        printf("Increasing vector capacity from %d to %d\n", v->capacity, new_capacity);
        v->capacity = new_capacity;
    }

    memcpy(v->data + (v->size++ << v->elt_magn), elt, 1 << v->elt_magn);
}

void swap_vectors(Vector *a, Vector *b)
{
    Vector temp = *a;
    *a = *b;
    *b = temp;
}

void cleanup_vector(Vector *v)
{
    if (v->data)
    {
        free(v->data);
        v->size = v->capacity = v->memsize = 0;
    }
}
