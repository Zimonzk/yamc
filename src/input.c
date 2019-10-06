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

#include <SDL2/SDL.h>
#include <string.h>

static struct keystates ks = {};
static SDL_bool game_pause = SDL_FALSE;
static struct confstate keyconfig;
static arraylist controls;

extern char inreach;
extern struct longpos player_lpos;
extern float looked_at[3];


void init_controls()
{
	arraylist_init(&controls, sizeof(struct control_element), 1);
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
	struct control_element celem;
	char *key_key, *key_mod;
	char *val_key = 0, *val_mod = 0;
	key_key = malloc(strlen(name) + 5); /* _key\0 */
	key_mod = malloc(strlen(name) + 5); /* _mod\0 */
	if(key_mod == 0 || key_key == 0) {
		yamc_terminate(ENOMEM, "Failed to allocate memory.");
	}
	sprintf(key_key, "%s%s", name, "_key");
	sprintf(key_mod, "%s%s", name, "_mod");
	conf_register_key(&keyconfig, key_key, (key_callback)kkcb, &val_key);
	conf_register_key(&keyconfig, key_mod, (key_callback)kmcb, &val_mod);

	conf_parse_file(&keyconfig, "config/controls.conf");

	conf_destroy_state(&keyconfig);


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

struct keystates* get_keystates(void) {
	return &ks;
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
