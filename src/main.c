/** Entry point and control code. */

#include <stdio.h>
#include <SDL.h>

#include "draw.h"
#include "game.h"

void poll_events()
{
    SDL_Event e;
    while (1)
    {
        if (SDL_WaitEvent(&e))
        {
            if (e.type == SDL_QUIT)
                return;
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)
                return;
        }
    }
}

int main(int argc, char **argv)
{
    if (init_game())
        return 1;

    poll_events();

    cleanup_draw();
    return 0;
}
