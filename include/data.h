#ifndef DATA_H
#define DATA_H

typedef char Bitboard[8];
extern const Bitboard FULL_BOARD;

#define square_file(s) ((s) & 7)
#define square_rank(s) ((s) >> 3)

typedef struct {
  char src_square, dest_square;
} Move;

typedef struct {
  void *data;
  int size, capacity, memsize;
  char elt_magn;
} Vector;

typedef struct {
  char *bytes;
  int size, capacity;
} ByteSet;

void clear_board(Bitboard board);
char board_empty(const Bitboard board);
void set_square(int square, Bitboard board);
void calc_board_overlap(const Bitboard a, const Bitboard b, Bitboard overlap);

void init_vector(Vector *vector, int elt_size, char capacity);
void *vector_get(Vector *vector, char index);
void vector_append(Vector *vector, void *elt);
void swap_vectors(Vector *a, Vector *b);
void *choose_random_elt(Vector *v);
void cleanup_vector(Vector *v);
#define clear_vector(v) ((v)->size = 0)

void init_set(ByteSet *set, char capacity);
void set_add(ByteSet *set, char elt);
char set_contains(ByteSet *set, char elt);
void cleanup_set(ByteSet *set);
#define clear_set(s) ((s)->size = 0)

#endif // DATA_H
