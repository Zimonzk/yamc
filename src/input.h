#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <SDL2/SDL.h>

#define DIRECTION_UP 1 << 0
#define DIRECTION_DOWN 1 << 1
#define DIRECTION_FORWARD 1 << 2
#define DIRECTION_BACKWARD 1 << 3
#define DIRECTION_LEFT 1 << 4
#define DIRECTION_RIGHT 1 << 5

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

char get_movement_directions(void);

void pause_game();
void unpause_game();

#endif // INPUT_H_INCLUDED
