#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <SDL2/SDL.h>

struct keystates {
    char UP;
    char DOWN;
};

enum keypress {KEY_UP, KEY_DOWN};

typedef void (*control_callback)(char *name, enum keypress kp, void *userdata);

struct control_element {
	char *name;
	control_callback ccb;
	void *userdata;
	SDL_Keycode keycode;
	SDL_Keymod keymod;
};


void init_controls();
void add_control_key(char *name, control_callback ccb, void * userdata,
		SDL_Keycode default_keycode, SDL_Keymod default_keymod);

void handle_keyboard_event(SDL_KeyboardEvent* kevent);

void handle_mousemotion_event(SDL_MouseMotionEvent* mvevent);

void handle_mousebutton_event(SDL_MouseButtonEvent* bevent);

struct keystates* get_keystates(void);

#endif // INPUT_H_INCLUDED
