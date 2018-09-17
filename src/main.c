/*
 * http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/* If using gl3.h */
/* Ensure we are using opengl's core profile only */
#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/gl.h>

//#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#define PROGRAM_NAME "Tutorial1"

#include "dndndefs.h"
#include "shader.h"
#include "matr.h"
#include "texture.h"
#include "input.h"
#include "rendering.h"
#include "worldgen.h"
#include "player.h"

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

chunk world[(2*CHUNK_LOADING_RANGE)+1][(2*CHUNK_LOADING_RANGE)+1] = {};
chunk* neig[4] = {0,0,0,0};

/*for simplex noise*/
struct osn_context* ctn;

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

    memset(world[0][0].data, 0, CHUNK_LIM_HOR*CHUNK_LIM_HOR*CHUNK_LIM_VER*sizeof(block));
    /*world[0][0].data[0][0][1].id = 2;
    world[0][0].data[0][0][1].properties = BLOCK_OPAQUE;
    world[0][0].data[0][0][0].id = 2;
    world[0][0].data[0][0][0].properties = BLOCK_OPAQUE;
    world[0][0].data[0][1][0].id = 2;
    world[0][0].data[0][1][0].properties = BLOCK_OPAQUE;
    world[0][0].data[1][0][0].id = 2;
    world[0][0].data[1][0][0].properties = BLOCK_OPAQUE;*/
    //fillchunktest(&world[0][0]);

    open_simplex_noise(TEST_SEED, &ctn);
    for(int x = 0; x < ((2*CHUNK_LOADING_RANGE) + 1); x++) {
        for(int z = 0; z < ((2*CHUNK_LOADING_RANGE) + 1); z++) {
            memset(world[x][z].data, 0, CHUNK_LIM_HOR*CHUNK_LIM_HOR*CHUNK_LIM_VER*sizeof(block));
            world[x][z].offset_X = x;
            world[x][z].offset_Z = z;
            generate_chunk(&world[x][z]);
        }
    }

    SDL_Log("#Triangles@world[0][0]: %i", determine_mescha_size(&world[0][0], neig));

    render_init();

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
          default:
            break;
          }
        }

        move_player(difftime);

        /*for(int x = 0; x < ((2*CHUNK_LOADING_RANGE) + 1); x++) {
            for(int z = 0; z < ((2*CHUNK_LOADING_RANGE) + 1); z++) {
                render_chunk(&(world[x][z]));
                //SDL_Log("rendered chunk");
            }
        }*/
        render_chunk(&(world[0][0]));
        //render_chunk(&(world[1][1]));

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
