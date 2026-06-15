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
    Move *moves;
    int size, capacity, memsize;
} MoveVector;

void clear_board(Bitboard board);
char board_empty(const Bitboard board);
void set_square(int square, Bitboard board);
void calc_board_overlap(const Bitboard a, const Bitboard b, Bitboard overlap);

void init_vector(MoveVector *vector);
Move *vector_get(MoveVector *vector, char index);
void add_to_vector(MoveVector *vector, Move move);
void swap_vectors(MoveVector *a, MoveVector *b);
void cleanup_vector(MoveVector *v);

#define clear_vector(v) ((v)->size = 0)

#endif // DATA_H
