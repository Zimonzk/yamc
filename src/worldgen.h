#ifndef WORLDGEN_H_INCLUDED
#define WORLDGEN_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <SDL2/SDL.h>

#include "open-simplex-noise.h"
#include "dndndefs.h"
#include "chunk.h"

void generate_chunk(chunk* chu, struct osn_context* ctn);

#endif // WORLDGEN_H_INCLUDED