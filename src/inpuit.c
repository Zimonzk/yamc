#include "input.h"
#include "player.h"
#include <SDL2/SDL.h>

static struct keystates ks = {};
static SDL_bool pause = SDL_FALSE;

void handle_keyboard_event(SDL_KeyboardEvent* kevent)
{
    switch(kevent->keysym.sym) {
    case SDLK_ESCAPE:
        if(kevent->type == SDL_KEYDOWN) {
            pause = !pause;
            SDL_SetRelativeMouseMode(!pause);
        }
        break;
    case SDLK_UP:
        ks.UP = kevent->type == SDL_KEYDOWN ? 1 : 0;
        //SDL_Log("UP: %i", (int) ks.UP);
        break;
    case SDLK_DOWN:
        ks.DOWN = kevent->type == SDL_KEYDOWN ? 1 : 0;
        break;
    }
}

void handle_mousemotion_event(SDL_MouseMotionEvent* mvevent)
{
    player_turn(mvevent);
}

struct keystates* get_keystates(void) {
    return &ks;
}
