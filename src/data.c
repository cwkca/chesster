/* Data structures. */

#include "data.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_INC_MAGN 3

const Bitboard FULL_BOARD = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/*
 * Bitboards
 */

void clear_board(Bitboard board) { memset(board, 0, 8); }

char board_empty(const Bitboard board) {
  int rank;
  for (rank = 0; rank < 8; rank++)
    if (board[rank])
      return 0;
  return 1;
}

void set_square(int square, Bitboard board) {
  board[square_rank(square)] |= 1 << square_file(square);
}

void calc_board_overlap(const Bitboard a, const Bitboard b, Bitboard overlap) {
  int rank;
  for (rank = 0; rank < 8; rank++)
    overlap[rank] = a[rank] & b[rank];
}

/*
 * Vectors
 */

void init_vector(Vector *v, int elt_size, char capacity) {
  assert(elt_size >= 0 && capacity > 0);

  for (v->elt_magn = 0, elt_size--; elt_size; v->elt_magn++)
    elt_size >>= 1;

  v->memsize = capacity * (1 << v->elt_magn);
  v->data = malloc(v->memsize);
  if (!v->data) {
    printf("Unable to allocate %d bytes of memory\n", v->memsize);
    exit(1);
  }

  v->size = 0;
  v->capacity = capacity;
}

void *vector_get(Vector *v, char i) {
  if (i < 0 || i >= v->size) {
    printf("Vector index %d out of bounds (size %d)\n", i, v->size);
    exit(1);
  }

  return v->data + (i << v->elt_magn);
}

void vector_append(Vector *v, void *elt) {
  assert(v->size <= v->capacity);

  if (v->size == v->capacity) {
    int new_memsize = v->memsize + (1 << (v->elt_magn + DATA_INC_MAGN));
    void *new_data = realloc(v->data, new_memsize);
    if (!new_data) {
      printf("Unable to allocate %d bytes of memory\n", new_memsize);
      exit(1);
    }

    v->data = new_data;
    v->memsize = new_memsize;

    int new_capacity = v->capacity + (1 << DATA_INC_MAGN);
    printf("Increasing vector capacity from %d to %d\n", v->capacity,
           new_capacity);
    v->capacity = new_capacity;
  }

  memcpy(v->data + (v->size++ << v->elt_magn), elt, 1 << v->elt_magn);
}

void swap_vectors(Vector *a, Vector *b) {
  Vector temp = *a;
  *a = *b;
  *b = temp;
}

void *choose_random_elt(Vector *v) {
  return v->size ? vector_get(v, rand() % v->size) : NULL;
}

void cleanup_vector(Vector *v) {
  if (v->data) {
    free(v->data);
    v->data = NULL;
    v->size = v->capacity = v->memsize = 0;
  }
}

/*
 * Byte sets
 */

void init_set(ByteSet *set, char capacity) {
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

  assert(set->size <= set->capacity);
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
  }

  set->bytes[set->size++] = elt;
}

char set_contains(ByteSet *set, char elt) {
  int i;
  for (i = 0; i < set->size; i++)
    if (set->bytes[i] == elt)
      return 1;
  return 0;
}

void cleanup_set(ByteSet *set) {
  if (set->bytes) {
    free(set->bytes);
    set->bytes = NULL;
    set->size = set->capacity = 0;
  }
}
