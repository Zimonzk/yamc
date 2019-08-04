#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
/* If using gl3.h */
/* Ensure we are using opengl's core profile only */
#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

//#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#define PROGRAM_NAME "YAMC"

#include "dndndefs.h"
#include "shader.h"
#include "matr.h"
#include "texture.h"
#include "input.h"
#include "rendering.h"
#include "worldgen.h"
#include "player.h"
#include "world.h"
#include "entity.h"
#include "longpos.h"
#include "fonter.h"
#include "confconfig.h"
#include "gui.h"
#include "event.h"

#define NUMVERT 36

int debug = 1;

/* A simple function that prints a message, the error code returned by SDL,
 * and quits the application */
void sdldie(const char *msg)
{
	SDL_Log("%s: %s\n", msg, SDL_GetError());
	SDL_Quit();
	exit(1);
}


chunk* neig[4] = {(chunk*)0, (chunk*)0, (chunk*)0, (chunk*)0};

extern struct longpos player_lpos;
extern float looked_at[3];
extern char inreach;

void ontestoption(char *value, void *userdata)
{
	SDL_Log("Testvalue = \"%s\"", value);
}

void ontestbutton(void* userdata)
{
	struct event_index_card *ic = userdata;
	SDL_Log("ACTION");
	trigger_event(ic, "came from testbutton! xD");
	//SDL_Quit();
	//exit(0);
}

void ontestevent(const struct event_index_card * ic,
				void* eventdata, void * userdata)
{
	SDL_Log("Eventdata: %s\n", (char *)eventdata);
	SDL_Log("Userdata: %s\n", (char *)userdata);

	frame_sync_begin();
	SDL_Quit();
	exit(0);
	frame_sync_end();
}


/* Our program's entry point */
int main(int argc, char *argv[])
{
	char stop = 0;
	SDL_Event event;

	Uint32 lastticks = 0, thisticks = 0, difftime = 0;
	Uint32 fpsticks = 0, fpslastticks = 0;
	float fps = 0;
	long frame_num = 0;
	char fpsstr[64];

	struct confstate confstate = {};

	SDL_Window *mainwindow; /* Our window handle */
	SDL_GLContext maincontext; /* Our opengl context handle */

	unsigned int side_texi, top_texi;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) { /* Initialize SDL's Video subsystem */
		sdldie("Unable to initialize SDL"); /* Or die on error */
	}
	SDL_Log("SDL initialized");

	/* Request opengl 3.2 context.
	 * SDL doesn't have the ability to choose which profile at this time of writing,
	 * but it should default to the core profile */
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	/* Turn on double buffering with a 24bit Z buffer.
	 * You may need to change this to 16 or 32 for your system */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	/* Create our window centered at 512x512 resolution */
	mainwindow = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!mainwindow) /* Die if creation failed */
		sdldie("Unable to create window");

	/* Create our opengl context and attach it to our window */
	maincontext = SDL_GL_CreateContext(mainwindow);
	
	/* init glew */
	glewExperimental = GL_TRUE;
	glewInit();

	/* load some block textures to texture our block with */
	side_texi = load_block_texture("textures/blocks/side.png");
	top_texi = load_block_texture("textures/blocks/top.png");
	/* create a block */
	register_block("soil", "Dirt", 1, (const unsigned int[6]) {side_texi, side_texi, side_texi, side_texi, top_texi, top_texi});

	conf_register_key(&confstate, "testoption", ontestoption, NULL);
	if(conf_parse_file(&confstate, "config/options.conf") == CONFCONFIG_ERROR_NOFILE) {
		sdldie("Invalid config file");
	}

	render_init();
	init_entities();
	initfont();
	init_gui();

	{
		struct event_index_card ic = {.name = "sys.TEST"};
		struct event_index_card *ic_reg = register_event(&ic);
		register_event_handler("sys.TEST", ontestevent, "is user data! :)");
		/*test button*/
		gui_add_button(-0.5f, -0.5f, 0.5f, 96.0f/480.0f, "TEST", ontestbutton, (void *)ic_reg);
	}		

	{
		struct entity_index_card card;
		struct entity_index_card *eic;
		struct live_entity *lep;
		long chunk[2] = {0, 0};
		double pos[3] = {20.0, 12.0, 20.0};

		card.name = strdup("zimonzk.test");
		card.displayname = strdup("TEST");
		card.health = -1;
		card.rt = RENDER_SPRITE;
		card.em.sp.size = 0.5f;
		card.em.sp.textureID = texture_from_png("textures/blocks/stone.png");
		if(card.em.sp.textureID == 0) {
			sdldie("no texture for entity");
		}
	       
		eic = register_entity(&card);
		
		lep = spawn_entity(eic, chunk, pos);
	}

	SDL_Log("Entering main loop");

	SDL_SetRelativeMouseMode(SDL_TRUE);

	/* main loop */
	while(!stop) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
				case SDL_KEYUP:
					handle_keyboard_event(&event.key);
					break;
				case SDL_QUIT:
					stop = 1;
					break;
				case SDL_MOUSEMOTION:
					handle_mousemotion_event(&event.motion);
					break;
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					handle_mousebutton_event(&event.button);
					break;
				default:
					break;
			}
		}

		move_player(difftime);

		render_looper();

		render_text("Hello world!", 0.0f, 0.0f);
		snprintf(fpsstr, 64, "%.1f FPS", fps);
		render_text(fpsstr, -1.0f, 1.0f - (32.0f/480.0f));

		/* allows events to synchronize to the main loop at this point. */
		main_loop_sync_slot();


		/* buffer swap*/
		SDL_GL_SwapWindow(mainwindow);
		thisticks = SDL_GetTicks();
		difftime = thisticks - lastticks;
		lastticks = thisticks;

		if(!(frame_num % 16)) {
			fpsticks = SDL_GetTicks();
			fps = 16000.0/(fpsticks - fpslastticks);
			fpslastticks = fpsticks;
			//SDL_Log("%f", fps);
		}		
		frame_num++;
	}

	/* Delete our opengl context, destroy our window, and shutdown SDL */
	SDL_GL_DeleteContext(maincontext);
	SDL_DestroyWindow(mainwindow);
	SDL_Quit();

	return 0;
}
