#ifndef CHUNK_H_INCLUDED
#define CHUNK_H_INCLUDED

#include "block.h"

#define CHUNK_LIM_HOR 64
#define CHUNK_LIM_VER 1024

#define CHUNK_LOADED 0b1

#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <SDL2/SDL.h>

typedef struct s_chunk
{
  block data[CHUNK_LIM_HOR][CHUNK_LIM_VER][CHUNK_LIM_HOR]; /*x, y, z of a block*/
  long offset_X, offset_Z;
  unsigned char state;
} chunk;

Uint32 blocks_filled(chunk* cchunk);

Uint32 determine_mescha_size(chunk* cchunk, chunk* neighbours[4]);
Uint32 generate_mescha(chunk* cchunk, chunk* neighbours[4], GLuint vertexbuffer, GLuint texibuffer);

#endif // CHUNK_H_INCLUDED
