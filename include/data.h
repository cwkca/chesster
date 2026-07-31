#ifndef DATA_H
#define DATA_H

typedef char Bitboard[8];
extern const Bitboard FULL_BOARD;

#define square_file(s) ((s) & 7)
#define square_rank(s) ((s) >> 3)

typedef struct {
  char src_square, dest_square;
} Move;

#define SPECIAL_MOVE 0x40

typedef struct {
  void *data;
  int size, capacity, elt_size;
} Vector;

typedef struct {
  char *bytes;
  int size, capacity;
} ByteSet;

typedef struct {
  int size, slot_mask, key_size, val_size;
  Vector keys, values;
} HashMap;

void clear_board(Bitboard board);
char board_empty(const Bitboard board);
char has_square(const Bitboard board, char square);
void add_board_to_vector(char src_square, const Bitboard move_board,
                         Vector *move_vector);
void calc_board_overlap(const Bitboard a, const Bitboard b, Bitboard overlap);

void vector_init(Vector *vector, int elt_size, int capacity);
void *vector_get(const Vector *vector, int index);
void vector_resize(Vector *vector, int size);
void vector_append(Vector *vector, void *elt);
void vector_zero(Vector *vector);
void vector_swap(Vector *a, Vector *b);
void *vector_random_elt(const Vector *v);
void vector_cleanup(Vector *v);
#define vector_clear(v) ((v)->size = 0)
#define vector_empty(v) ((v)->size == 0)

void set_init(ByteSet *set, char capacity);
void set_add(ByteSet *set, char elt);
char set_contains(const ByteSet *set, char elt);
void set_cleanup(ByteSet *set);
#define set_clear(s) ((s)->size = 0)

void map_init(HashMap *map, int key_size, int value_size, int capacity);
int map_put(HashMap *map, const void *key, const void *value);
void *map_get(const HashMap *map, const void *key);
void map_clear(HashMap *map);
void map_cleanup(HashMap *map);

#endif // DATA_H
