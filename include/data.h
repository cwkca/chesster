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
  int size, capacity, memsize;
  char elt_magn;
} Vector;

typedef struct {
  char *bytes;
  int size, capacity;
} ByteSet;

void clear_board(Bitboard board);
char board_empty(const Bitboard board);
char has_square(Bitboard board, char square);
void add_board_to_vector(char src_square, Bitboard move_board,
                         Vector *move_vector);
void calc_board_overlap(const Bitboard a, const Bitboard b, Bitboard overlap);

void vector_init(Vector *vector, int elt_size, int capacity);
void *vector_get(Vector *vector, int index);
void vector_append(Vector *vector, void *elt);
void vector_swap(Vector *a, Vector *b);
void *vector_random_elt(Vector *v);
void vector_cleanup(Vector *v);
#define vector_clear(v) ((v)->size = 0)
#define vector_empty(v) ((v)->size == 0)

void set_init(ByteSet *set, char capacity);
void set_add(ByteSet *set, char elt);
char set_contains(ByteSet *set, char elt);
void set_cleanup(ByteSet *set);
#define set_clear(s) ((s)->size = 0)

#endif // DATA_H
