#ifndef DRAW_H
#define DRAW_H

#include <SDL.h>

extern SDL_Color square_highlights[64];

typedef struct
{
    int (*draw_screen)();
    int (*draw_board)();
    void (*cleanup)();
} DrawAdapter;

DrawAdapter *init_draw();

void highlight_board(DrawAdapter *, SDL_Color color, int (*highlight)(int, int));

#endif /* DRAW_H */
