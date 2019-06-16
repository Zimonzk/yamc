#include "input.h"
#include "player.h"
#include "longpos.h"
#include "world.h"
#include "rendering.h"
#include "gui.h"

#include <SDL2/SDL.h>

static struct keystates ks = {};
static SDL_bool pause = SDL_FALSE;

extern char inreach;
extern struct longpos player_lpos;
extern float looked_at[3];

void handle_keyboard_event(SDL_KeyboardEvent* kevent)
{
	static int mouse_reset_pos[2] = {0, 0};
	switch(kevent->keysym.sym) {
		case SDLK_ESCAPE:
			if(kevent->type == SDL_KEYDOWN) {
				pause = !pause;
				SDL_SetRelativeMouseMode(!pause);
				if(pause) {
					/*TODO onPause event*/
					SDL_WarpMouseInWindow(NULL,
							mouse_reset_pos[0],
							mouse_reset_pos[1]);

				} else {
					/*TODO onUnpause event*/
					SDL_GetMouseState(&mouse_reset_pos[0],
							&mouse_reset_pos[1]);
				}
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
	if(!pause) {
		player_turn(mvevent);
	}
}


void handle_mousebutton_event(SDL_MouseButtonEvent* bevent)
{
	switch(bevent->button) {
		case SDL_BUTTON_LEFT:
		if(bevent->type == SDL_MOUSEBUTTONDOWN) {
			/*button pressed*/
			if(pause) {
				gui_input(bevent->x, bevent->y, 1);
			}
			if(inreach && !pause) {
				struct longpos lpos;
				rrpos_to_lpos(looked_at, player_lpos, lpos);
				chunk* mchunk = world(lpos.chunk[0], lpos.chunk[1]);
				mchunk->data[(int)lpos.rpos[0]][(int)lpos.rpos[1]][(int)lpos.rpos[2]].id = 0;
				mchunk->data[(int)lpos.rpos[0]][(int)lpos.rpos[1]][(int)lpos.rpos[2]].properties = 0;
				/* TODO check if the chunk is loaded (has mesh) */
				update_mesh_abs(lpos.chunk[0], lpos.chunk[1]);
				SDL_Log("CLICK! %f|%f|%f", looked_at[0], looked_at[1], looked_at[2]);
				SDL_Log("~~~~~~ %i|%i|%i", (int)floor(looked_at[0]), (int)looked_at[1], (int)floor(looked_at[2]));
			} else {
				SDL_Log("Not looking at a block in reach or game paused.");
			}
		} else {
			/*button released*/
			if(pause) {
				gui_input(bevent->x, bevent->y, 0);
			}
		}
		break;
	}
}

struct keystates* get_keystates(void) {
	return &ks;
}
