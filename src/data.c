/* Data structures. */

#include "data.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const Bitboard FULL_BOARD = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/*
 * Bitboards
 */

void clear_board(Bitboard board) { memset(board, 0, 8); }

char board_empty(const Bitboard board) {
  char rank;
  for (rank = 0; rank < 8; rank++)
    if (board[rank])
      return 0;
  return 1;
}

char has_square(Bitboard board, char square) {
  return board[square_rank(square)] & (1 << square_file(square));
}

void add_board_to_vector(char src_square, Bitboard move_board,
                         Vector *move_vector) {
  char rank, square, mask;
  Move move;
  for (rank = square = 0; rank < 8; rank++)
    for (mask = 1; mask; mask <<= 1, square++)
      if (move_board[rank] & mask) {
        move = (Move){src_square, square};
        vector_append(move_vector, &move);
      }
}

void calc_board_overlap(const Bitboard a, const Bitboard b, Bitboard overlap) {
  char rank;
  for (rank = 0; rank < 8; rank++)
    overlap[rank] = a[rank] & b[rank];
}

/*
 * Vectors
 */

void vector_init(Vector *v, int elt_size, int capacity) {
  assert(elt_size >= 0 && capacity > 0);

  v->elt_size = elt_size;
  int memsize = capacity * elt_size;
  v->data = malloc(memsize);
  if (!v->data) {
    printf("Unable to allocate %d bytes of memory\n", memsize);
    exit(1);
  }

  v->size = 0;
  v->capacity = capacity;
}

void *vector_get(Vector *v, int i) {
  if (i < 0 || i >= v->size) {
    printf("Vector index %d out of bounds (size %d)\n", i, v->size);
    exit(1);
  }

  return v->data + (i * v->elt_size);
}

void vector_append(Vector *v, void *elt) {
  int last;
  assert(v->size <= v->capacity);

  if (v->size == v->capacity) {
    int memsize = v->size * v->capacity << 1;
    v->data = realloc(v->data, memsize);
    if (!v->data) {
      printf("Unable to allocate %d bytes of memory\n", memsize);
      exit(1);
    }

    v->capacity <<= 1;
    printf("Doubling vector capacity to %d\n", v->capacity);
  }

  last = v->size++;
  memcpy(vector_get(v, last), elt, v->elt_size);
}

void vector_swap(Vector *a, Vector *b) {
  Vector temp = *a;
  *a = *b;
  *b = temp;
}

void *vector_random_elt(Vector *v) {
  return v->size ? vector_get(v, rand() % v->size) : NULL;
}

void vector_cleanup(Vector *v) {
  if (v->data) {
    free(v->data);
    v->data = NULL;
    v->size = v->capacity = v->elt_size = 0;
  }
}

/*
 * Byte sets
 */

void set_init(ByteSet *set, char capacity) {
  assert(capacity > 0);

  set->bytes = malloc(capacity);
  if (!set->bytes) {
    printf("Unable to allocate %d bytes of memory\n", capacity);
    exit(1);
  }

  set->size = 0;
  set->capacity = capacity;
}

void set_add(ByteSet *set, char elt) {
  if (set_contains(set, elt))
    return;

  assert(set->size < set->capacity);
  /* Sets should not need resizing
  if (set->size == set->capacity) {
    int new_capacity = set->capacity + (1 << DATA_INC_MAGN);
    char *new_bytes = realloc(set->bytes, new_capacity);
    if (!new_bytes) {
      printf("Unable to allocate %d bytes of memory\n", new_capacity);
      exit(1);
    }

    printf("Increasing set capacity from %d to %d\n", set->capacity,
           new_capacity);
    set->bytes = new_bytes;
    set->capacity = new_capacity;
  } */

  set->bytes[set->size++] = elt;
}

char set_contains(ByteSet *set, char elt) {
  int i;
  for (i = 0; i < set->size; i++)
    if (set->bytes[i] == elt)
      return 1;
  return 0;
}

void set_cleanup(ByteSet *set) {
  if (set->bytes) {
    free(set->bytes);
    set->bytes = NULL;
    set->size = set->capacity = 0;
  }
}
