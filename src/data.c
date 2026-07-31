/* Data structures. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "debug.h"

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

char has_square(const Bitboard board, char square) {
  return board[square_rank(square)] & (1 << square_file(square));
}

void add_board_to_vector(char src_square, const Bitboard move_board,
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
  if (!v->data)
    throw("Unable to allocate memory");

  v->size = 0;
  v->capacity = capacity;
}

void *vector_get(const Vector *v, int i) {
  if (i < 0 || i >= v->size)
    throw("Vector index out of bounds");

  return v->data + (i * v->elt_size);
}

void vector_resize(Vector *v, int size) {
  assert(size >= 0);
  v->size = size;
  if (size <= v->capacity)
    return;

  while (size > v->capacity)
    v->capacity <<= 1;

  int memsize = v->elt_size * v->capacity;
  v->data = realloc(v->data, memsize);
  if (v->data)
    printf("Increasing vector capacity to %d\n", v->capacity);
  else
    throw("Unable to allocate memory");
}

void vector_append(Vector *v, void *elt) {
  int last = v->size++;
  vector_resize(v, v->size);
  memcpy(vector_get(v, last), elt, v->elt_size);
}

void vector_zero(Vector *v) { memset(v->data, 0, v->elt_size * v->size); }

void vector_swap(Vector *a, Vector *b) {
  Vector temp = *a;
  *a = *b;
  *b = temp;
}

void *vector_random_elt(const Vector *v) {
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
  if (!set->bytes)
    throw("Unable to allocate memory");

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

char set_contains(const ByteSet *set, char elt) {
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

/*
 * Maps
 */

#define HASH_BYTES 4
#define LOAD_MAGN 1
#define MAP_MAX 2e6
typedef uint32_t dword;

void init_slots(Vector *vec, int slot_size, int count) {
  vector_init(vec, slot_size, count);
  vector_resize(vec, count);
  vector_zero(vec);
}

dword rotate_left(const dword *val, int bits) {
  return (*val << bits) | (*val >> (32 - bits));
}

dword calc_hash(const void *val, int bytes) {
  int bit, byte;
  const char *data = val;
  dword hash;
  const void *end = val + bytes;

  if (bytes < HASH_BYTES) {
    for (hash = byte = 0; byte < bytes; byte++) {
      hash |= *data++;
      hash <<= 8;
    }
    return hash;
  }

  hash = 0x811C9DC5;
  for (end = val + bytes - 4; val < end; val += 4) {
    hash ^= *((dword *)val);
    hash *= 0x01000193;
  }
  hash ^= *((dword *)(end - 4));

  /* Zero byte marks empty spots, so hashes cannot start with that. */
  if (!(hash & 0xFF))
    hash++;

  return hash;
}

int slot_insert(HashMap *map, dword hash, const void *key, const void *value) {
  void *entry;
  int slot = hash & map->slot_mask;

  /* Advance to the first empty slot */
  int skips = 0;
  for (entry = vector_get(&map->keys, slot); *((const char *)entry);
       entry = vector_get(&map->keys, ++slot), skips++)
    if (slot == map->keys.size - 1)
      slot = -1;

  memcpy(entry, &hash, HASH_BYTES);
  memcpy(entry + HASH_BYTES, key, map->key_size);
  memcpy(vector_get(&map->values, slot), value, map->val_size);
  return skips;
}

void map_expand(HashMap *map) {
  const void *entry;
  Vector old_keys = map->keys, old_values = map->values;

  int slot, new_size = map->keys.size << 1;
  // printf("Expanding map to %d slots\n", new_size);
  init_slots(&map->keys, HASH_BYTES + map->key_size, new_size);
  init_slots(&map->values, map->val_size, new_size);
  map->slot_mask <<= 1;
  map->slot_mask |= 1;

  for (slot = 0; slot < old_keys.size; slot++) {
    entry = vector_get(&map->keys, slot);
    if (*((const char *)entry))
      slot_insert(map, *((dword *)entry), entry + HASH_BYTES,
                  vector_get(&old_values, slot));
  }

  vector_cleanup(&old_keys);
  vector_cleanup(&old_values);
}

void map_init(HashMap *map, int key_size, int val_size, int capacity) {
  int table_size;
  for (table_size = 1; table_size < capacity; table_size <<= 1)
    ;
  init_slots(&map->keys, HASH_BYTES + key_size, table_size);
  init_slots(&map->values, val_size, table_size);

  map->slot_mask = table_size - 1;
  map->key_size = key_size;
  map->val_size = val_size;
  map->size = 0;
}

int map_put(HashMap *map, const void *key, const void *value) {
  int hash, slot, max_load = map->keys.size - (map->keys.size >> LOAD_MAGN);
  void *entry;

  if (++map->size > MAP_MAX) {
    // printf("Emptying move cache\n");
    map_clear(map);
  } else if (map->size > max_load)
    map_expand(map);

  return slot_insert(map, calc_hash(key, map->key_size), key, value);
}

void *map_get(const HashMap *map, const void *key) {
  int slot = calc_hash(key, map->key_size) & map->slot_mask;
  const void *entry;

  for (entry = vector_get(&map->keys, slot); *((const char *)entry);
       entry = vector_get(&map->keys, ++slot)) {
    if (memcmp(key, entry + HASH_BYTES, map->key_size) == 0)
      return vector_get(&map->values, slot);
    else if (slot == map->keys.size - 1)
      slot = -1;
  }

  return NULL;
}

void map_clear(HashMap *map) {
  map->size = 0;
  vector_zero(&map->keys);
}

void map_cleanup(HashMap *map) {
  map->size = 0;
  vector_cleanup(&map->keys);
  vector_cleanup(&map->values);
}
