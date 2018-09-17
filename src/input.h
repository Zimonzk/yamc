#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <SDL2/SDL.h>

struct keystates {
    char UP;
    char DOWN;
};

void handle_keyboard_event(SDL_KeyboardEvent* kevent);

void handle_mousemotion_event(SDL_MouseMotionEvent* mvevent);

struct keystates* get_keystates(void);

#endif // INPUT_H_INCLUDED
