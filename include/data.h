#ifndef DATA_H
#define DATA_H

typedef char Bitboard[8];
extern const Bitboard FULL_BOARD;

#define square_file(s) ((s) & 7)
#define square_rank(s) ((s) >> 3)

typedef struct
{
    char src_square, dest_square;
} Move;

typedef struct
{
    void *data;
    int size, capacity, memsize;
    char elt_magn;
} Vector;

void clear_board(Bitboard board);
char board_empty(const Bitboard board);
void set_square(int square, Bitboard board);
void calc_board_overlap(const Bitboard a, const Bitboard b, Bitboard overlap);

void init_vector(Vector *vector, char elt_size);
void *vector_get(Vector *vector, char index);
void add_to_vector(Vector *vector, void *elt);
void swap_vectors(Vector *a, Vector *b);
void cleanup_vector(Vector *v);

#define clear_vector(v) ((v)->size = 0)

#endif // DATA_H
