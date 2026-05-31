#ifndef DRAW_H
#define DRAW_H

typedef struct
{
    int (*draw_board)();
    int (*draw_pieces)();
    void (*cleanup)();
} DrawAdapter;

DrawAdapter *init_draw();

#endif /* DRAW_H */
