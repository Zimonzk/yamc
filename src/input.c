#define _POSIX_C_SOURCE 200908L
#define _GNU_SOURCE

#include "input.h"
#include "player.h"
#include "longpos.h"
#include "world.h"
#include "rendering.h"
#include "gui.h"
#include "confconfig.h"
#include "toolbox.h"
#include "zimonzk/lists.h"
#include "menu.h"

#include <SDL2/SDL.h>
#include <string.h>

static char movement_directions = 0;
static SDL_bool game_pause = SDL_FALSE;
static arraylist controls;

extern char inreach;
extern struct longpos player_lpos;
extern float looked_at[3];

static void on_direction(char *name, enum keypress kp, void *userdata)
{
	if(kp == KEY_UP) {
		movement_directions &= ~(char)userdata;
	} else {
		movement_directions |= (char)userdata;
	}
}

void init_controls()
{
	arraylist_init(&controls, sizeof(struct control_element), 1);
	tlog(5, "Inited controls list.");
	/* TODO: only use one function and determine key via userdata.
	 * also use ond char for all directions and add actual up and down and
	 * rename what is now up and down to the proper forward and backward. */
	add_control_key("Up", (control_callback)on_direction,
			DIRECTION_UP, SDLK_KP_0, KMOD_NONE);
	add_control_key("Down", (control_callback)on_direction,
			DIRECTION_DOWN, SDLK_RSHIFT, KMOD_RSHIFT);
	add_control_key("Forward", (control_callback)on_direction,
			DIRECTION_FORWARD, SDLK_UP, KMOD_NONE);
	add_control_key("Backward", (control_callback)on_direction,
			DIRECTION_BACKWARD, SDLK_DOWN, KMOD_NONE);
	add_control_key("Left", (control_callback)on_direction,
			DIRECTION_LEFT, SDLK_LEFT, KMOD_NONE);
	add_control_key("Right", (control_callback)on_direction, 
			DIRECTION_RIGHT, SDLK_RIGHT, KMOD_NONE);
	tlog(5, "Inited default movement keybindings.");
}

static char *keymod_tostring(SDL_Keymod keymod);
static SDL_Keymod keymod_fromstring(char *name);

static void kkcb(char *value, char **key_value)
{
	*key_value = strdup(value);
}

static void kmcb(char *value, char **mod_value)
{
	*mod_value = strdup(value);
}

void add_control_key(char * name, control_callback ccb, void *userdata,
		SDL_Keycode default_keycode, SDL_Keymod default_keymod)
{
	struct confstate keyconfig = {};
	struct control_element celem = {};
	char *key_key, *key_mod;
	char *val_key = 0, *val_mod = 0;
	key_key = malloc(strlen(name) + 5); /* _key\0 */
	key_mod = malloc(strlen(name) + 5); /* _mod\0 */
	if(key_mod == 0 || key_key == 0) {
		yamc_terminate(ENOMEM, "Failed to allocate memory.");
	}
	tlog(5, "a0");
	sprintf(key_key, "%s%s", name, "_key");
	tlog(5, "a1");
	sprintf(key_mod, "%s%s", name, "_mod");
	tlog(5, "a2");
	conf_register_key(&keyconfig, key_key, (key_callback)kkcb, &val_key);
	tlog(5, "a3");
	conf_register_key(&keyconfig, key_mod, (key_callback)kmcb, &val_mod);
	tlog(5, "a4");

	conf_parse_file(&keyconfig, "config/controls.conf");

	tlog(5, "a5");

	conf_destroy_state(&keyconfig);

	tlog(5, "a6");


	if(val_key != 0) {
		/* key defined */
		SDL_Keycode kc = SDL_GetKeyFromName(val_key);
		if(kc == SDLK_UNKNOWN) {
			yamc_terminate(EINVAL, "Keycode not valid.");
		}
		default_keycode = kc;
		if(val_mod != 0) {
			/* key and modifier defined */
			default_keymod = keymod_fromstring(val_mod);
			free(val_mod);
		} else {
			/* modifier not defined, consider modifier as "none" */
			default_keymod = 0;
		}
		free(val_key);
	} else {
		/* key not defined */
		FILE* configf = fopen("config/controls.conf", "a");
		if(configf == 0) {
			yamc_terminate(errno, "Could not open config file for"
					" appending.");
		}
		fprintf(configf, "\n%s = %s", key_key,
				SDL_GetKeyName(default_keycode));
		if(val_mod != 0) {
			/* only modifier defined */
			default_keymod = keymod_fromstring(val_mod);
			free(val_mod);
		} else {
			/* none defined */
			/* use default an add default to file */
			char *kmstr = keymod_tostring(default_keymod);
			if(strcmp(kmstr, "") != 0) {
				fprintf(configf, "\n%s = %s", key_mod, kmstr);
			}
			free(kmstr);
		}
		fclose(configf);
	}

	celem.name = name;
	celem.ccb = ccb;
	celem.userdata = userdata;
	celem.keycode = default_keycode;
	celem.keymod = default_keymod;
	arraylist_append(&controls, &celem);

	free(key_key);
	free(key_mod);
}

void handle_keyboard_event(SDL_KeyboardEvent* kevent)
{
	static int mouse_reset_pos[2] = {0, 0};
	switch(kevent->keysym.sym) {
		case SDLK_ESCAPE:
			if(kevent->type == SDL_KEYDOWN) {
				game_pause = !game_pause;
				SDL_SetRelativeMouseMode(!game_pause);
				if(game_pause) {
					/*TODO onPause event*/
					SDL_WarpMouseInWindow(NULL,
							mouse_reset_pos[0],
							mouse_reset_pos[1]);
					build_esc_menu();

				} else {
					/*TODO onUnpause event*/
					SDL_GetMouseState(&mouse_reset_pos[0],
							&mouse_reset_pos[1]);
					gui_clear();
				}
			}
			break;
			/*case SDLK_UP:
			  ks.UP = kevent->type == SDL_KEYDOWN ? 1 : 0;
			//SDL_Log("UP: %i", (int) ks.UP);
			break;
			case SDLK_DOWN:
			ks.DOWN = kevent->type == SDL_KEYDOWN ? 1 : 0;
			break;*/
	}
	for(int i = 0; i < controls.used_units; i++) {
		struct control_element *celp =
			arraylist_get(&controls, i);
		/*tlog(5, "Checking for control nr. %i", i);
		  tlog(5, "got %i, have %i", kevent->keysym.sym, celp->keycode);*/

		if((celp->keycode == kevent->keysym.sym)) {
			uint16_t keymod_w, keymod_h, ig_keymod_w, ig_keymod_h;
			keymod_w = celp->keymod;
			keymod_h = kevent->keysym.mod;
			ig_keymod_w = keymod_w & ~(KMOD_NUM|KMOD_CAPS);
			ig_keymod_h = keymod_h & ~(KMOD_NUM|KMOD_CAPS);

			/*tlog(5, "w %i, h %i, iw %i, ih %i", keymod_w, keymod_h,
			  ig_keymod_w, ig_keymod_h);*/

			if(kevent->type == SDL_KEYUP
					|| (ig_keymod_w == ig_keymod_h)) {
				zlog(5, "Found corresponding control for input.");
				celp->ccb(	celp->name,
						(kevent->type == SDL_KEYDOWN) ?
						KEY_DOWN : KEY_UP,
						celp->userdata);
			}
		}
	}
}

void handle_mousemotion_event(SDL_MouseMotionEvent* mvevent)
{
	if(!game_pause) {
		player_turn(mvevent);
	}
}


void handle_mousebutton_event(SDL_MouseButtonEvent* bevent)
{
	switch(bevent->button) {
		case SDL_BUTTON_LEFT:
			if(bevent->type == SDL_MOUSEBUTTONDOWN) {
				/*button pressed*/
				if(game_pause) {
					gui_input(bevent->x, bevent->y, 1);
				}
				if(inreach && !game_pause) {
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
				if(game_pause) {
					gui_input(bevent->x, bevent->y, 0);
				}
			}
			break;
	}
}

char get_movement_directions(void) {
	return movement_directions;
}

static char *keymod_tostring(SDL_Keymod keymod)
{
	char *kmstr = malloc(1);
	if(kmstr == 0) {
		yamc_terminate(ENOMEM, "Failed to allocate keymod string.");
	}
	kmstr[0] = '\0';

	while(keymod != 0) {
		if(keymod & KMOD_LSHIFT) {
			keymod &= !KMOD_LSHIFT;
			kmstr = realloc(kmstr, strlen(kmstr) + 7);
			if(kmstr == 0) {
				yamc_terminate(ENOMEM, "Failed to allocate "
						"keymod string.");
			}
			strcat(kmstr, "LSHIFT+");
		} else if(keymod & KMOD_RSHIFT) {
			keymod &= !KMOD_RSHIFT;
			kmstr = realloc(kmstr, strlen(kmstr) + 7);
			if(kmstr == 0) {
				yamc_terminate(ENOMEM, "Failed to allocate "
						"keymod string.");
			}
			strcat(kmstr, "RSHIFT+");
		} else if(keymod & KMOD_LCTRL) {
			keymod &= !KMOD_LCTRL;
			kmstr = realloc(kmstr, strlen(kmstr) + 6);
			if(kmstr == 0) {
				yamc_terminate(ENOMEM, "Failed to allocate "
						"keymod string.");
			}
			strcat(kmstr, "LCTRL+");
		} else if(keymod & KMOD_RCTRL) {
			keymod &= !KMOD_RCTRL;
			kmstr = realloc(kmstr, strlen(kmstr) + 6);
			if(kmstr == 0) {
				yamc_terminate(ENOMEM, "Failed to allocate "
						"keymod string.");
			}
			strcat(kmstr, "RCTRL+");
		} else if(keymod & KMOD_LALT) {
			keymod &= !KMOD_LALT;
			kmstr = realloc(kmstr, strlen(kmstr) + 5);
			if(kmstr == 0) {
				yamc_terminate(ENOMEM, "Failed to allocate "
						"keymod string.");
			}
			strcat(kmstr, "LALT+");
		} else if(keymod & KMOD_RALT) {
			keymod &= !KMOD_RALT;
			kmstr = realloc(kmstr, strlen(kmstr) + 5);
			if(kmstr == 0) {
				yamc_terminate(ENOMEM, "Failed to allocate "
						"keymod string.");
			}
			strcat(kmstr, "RALT+");
		} else if(keymod & KMOD_LGUI) {
			keymod &= !KMOD_LGUI;
			kmstr = realloc(kmstr, strlen(kmstr) + 5);
			if(kmstr == 0) {
				yamc_terminate(ENOMEM, "Failed to allocate "
						"keymod string.");
			}
			strcat(kmstr, "LGUI+");
		} else if(keymod & KMOD_RGUI) {
			keymod &= !KMOD_RGUI;
			kmstr = realloc(kmstr, strlen(kmstr) + 5);
			if(kmstr == 0) {
				yamc_terminate(ENOMEM, "Failed to allocate "
						"keymod string.");
			}
			strcat(kmstr, "RGUI+");
		} else if(keymod & KMOD_NUM) {
			keymod &= !KMOD_NUM;
			kmstr = realloc(kmstr, strlen(kmstr) + 4);
			if(kmstr == 0) {
				yamc_terminate(ENOMEM, "Failed to allocate "
						"keymod string.");
			}
			strcat(kmstr, "NUM+");
		} else if(keymod & KMOD_CAPS) {
			keymod &= !KMOD_CAPS;
			kmstr = realloc(kmstr, strlen(kmstr) + 5);
			if(kmstr == 0) {
				yamc_terminate(ENOMEM, "Failed to allocate "
						"keymod string.");
			}
			strcat(kmstr, "CAPS+");
		} else if(keymod & KMOD_MODE) {
			keymod &= !KMOD_MODE;
			kmstr = realloc(kmstr, strlen(kmstr) + 5);
			if(kmstr == 0) {
				yamc_terminate(ENOMEM, "Failed to allocate "
						"keymod string.");
			}
			strcat(kmstr, "MODE+");
		}

		if(strlen(kmstr) > 0) {
			kmstr[strlen(kmstr) - 1] = '\0';
		}
	}

	return kmstr;
}

static SDL_Keymod keymod_fromstring(char *name)
{
	char *token;
	SDL_Keymod km = 0;

	while(token = strsep(&name, "+")) {
		if(strcmp(token, "LSHIFT") == 0) {
			km |= KMOD_LSHIFT;
		} else if(strcmp(token, "RSHIFT") == 0) {
			km |= KMOD_RSHIFT;
		} else if(strcmp(token, "LCTRL") == 0) {
			km |= KMOD_LCTRL;
		} else if(strcmp(token, "RCTRL") == 0) {
			km |= KMOD_RCTRL;
		} else if(strcmp(token, "LALT") == 0) {
			km |= KMOD_LALT;
		} else if(strcmp(token, "RALT") == 0) {
			km |= KMOD_RALT;
		} else if(strcmp(token, "LGUI") == 0) {
			km |= KMOD_LGUI;
		} else if(strcmp(token, "RGUI") == 0) {
			km |= KMOD_RGUI;
		} else if(strcmp(token, "NUM") == 0) {
			km |= KMOD_NUM;
		} else if(strcmp(token, "CAPS") == 0) {
			km |= KMOD_CAPS;
		} else if(strcmp(token, "MODE") == 0) {
			km |= KMOD_MODE;
		}
	}
	return km;
}
