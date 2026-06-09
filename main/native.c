/** Native entry point and event loop. */

#include <ctype.h>
#include <SDL.h>
#include "game.h"

void poll_events()
{
    SDL_Event event;
    while (1)
    {
        if (SDL_WaitEvent(&event))
        {
            if (event.type == SDL_QUIT)
                return;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)
                return;
            if (event.type == SDL_KEYDOWN)
                handle_key(event.key.keysym);
        }
    }
}

int main(int argc, char **argv)
{
    if (init_game() || start_game())
        return 1;

    poll_events();

    cleanup_game();
    return 0;
}
