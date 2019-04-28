/*
 * http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices
 *
 */
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
extern float looked_at[3];

/* Our program's entry point */
int main(int argc, char *argv[])
{
	char stop = 0;
	SDL_Event event;

	Uint32 lastticks = 0, thisticks = 0, difftime = 0;
	Uint32 fpsticks = 0, fpslastticks = 0;
	float fps = 0;
	long frame_num = 0;

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

	render_init();
	init_entities();

	{
		struct entity_index_card card;
		struct entity_index_card *eic;
		struct live_entity *lep;
		long chunk[2] = {0, 0};
		double pos[3] = {20.0, 20.0, 20.0};

		card.name = strdup("zimonzk.test");
		card.displayname = strdup("TEST");
		card.health = -1;
		card.rt = RENDER_SPRITE;
		card.em.sp.size = 20.0f;
		card.em.sp.textureID = texture_from_png("textures/blocks/stone.png");
	       
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
					handle_mousemotion_event(&event);
					break;
				case SDL_MOUSEBUTTONDOWN:
					{
						int cx = (int)floor(looked_at[0]/CHUNK_LIM_HOR);
						int cz = (int)floor(looked_at[2]/CHUNK_LIM_HOR);
						int crx = ((((int)floor(looked_at[0])) % CHUNK_LIM_HOR) + CHUNK_LIM_HOR) % CHUNK_LIM_HOR;
						int crz = ((((int)floor(looked_at[2])) % CHUNK_LIM_HOR) + CHUNK_LIM_HOR) % CHUNK_LIM_HOR;
						chunk* mchunk = world(cx, cz);
						mchunk->data[crx][(int)looked_at[1]][crz].id = 0;
						mchunk->data[crx][(int)looked_at[1]][crz].properties = 0;
						/* TODO check if the chunk is loaded (has mesh)*/
						update_mesh_abs(cx, cz);
						SDL_Log("CLICK! %f|%f|%f", looked_at[0], looked_at[1], looked_at[2]);
						SDL_Log("~~~~~~ %i|%i|%i", (int)floor(looked_at[0]), (int)looked_at[1], (int)floor(looked_at[2]));
					}
					break;
				default:
					break;
			}
		}

		move_player(difftime);

		render_looper();

		/* buffer swap*/
		SDL_GL_SwapWindow(mainwindow);
		thisticks = SDL_GetTicks();
		difftime = thisticks - lastticks;
		lastticks = thisticks;

		if(!(frame_num % 1000)) {
			fpsticks = SDL_GetTicks();
			fps = 1000000.0/(fpsticks - fpslastticks);
			SDL_Log("FPS: %f\n", fps);
			fpslastticks = fpsticks;
		}
		frame_num++;
	}

	/* Delete our opengl context, destroy our window, and shutdown SDL */
	SDL_GL_DeleteContext(maincontext);
	SDL_DestroyWindow(mainwindow);
	SDL_Quit();

	return 0;
}
