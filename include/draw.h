#ifndef DRAW_H
#define DRAW_H

typedef struct
{
    int (*draw_screen)();
    int (*draw_board)();
    void (*cleanup)();
} DrawAdapter;

DrawAdapter *init_draw();

#endif /* DRAW_H */
