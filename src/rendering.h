#ifndef RENDERING_H_INCLUDED
#define RENDERING_H_INCLUDED

#include "chunk.h"
#include <stdint.h>

void render_init();
void update_mesh(int x, int z);
void update_mesh_abs(long x, long z);
void render_looper();

void update_model();
void update_view();
void update_projection();
void update_mvp();

int pick_block(float *rrpos); /* if the player is looking at a block
			      * it writes the position of the block,
			      * the player is looking at into 3
			      * floats at rpos.
			      * returns whether the player is looking at a block
			      * at all. */

#endif // RENDERING_H_INCLUDED
