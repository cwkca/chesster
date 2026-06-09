/** Emscripten entry point and event loop. */

#include <SDL.h>
#include <emscripten.h>

#include "game.h"

void main_loop()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            emscripten_cancel_main_loop();
        if (event.type == SDL_KEYDOWN)
            handle_key(event.key.keysym);
    }
}

int main(int argc, char **argv)
{
    if (init_game() || start_game())
        return 1;

    // 0 fps means use requestAnimationFrame (recommended)
    emscripten_set_main_loop(main_loop, 0, 1);

    cleanup_game();
    return 0;
}
