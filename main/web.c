/** Emscripten entry point and event loop. */

#include <SDL.h>

#include "draw.h"
#include "game.h"

void main_loop()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            // wasm-ld: error: function signature mismatch (return type) :-(
            emscripten_cancel_main_loop();
    }
}

int main(int argc, char **argv)
{
    if (init_game())
        return 1;

    // 0 fps means use requestAnimationFrame (recommended)
    emscripten_set_main_loop(main_loop, 0, 1);

    cleanup_draw();
    return 0;
}
