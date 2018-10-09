#ifndef RENDERING_H_INCLUDED
#define RENDERING_H_INCLUDED

#include "chunk.h"
#include <stdint.h>

void render_init();
void update_mesh(int x, int z);
void update_mesh_abs(int x, int z);
void render_looper();

void update_model();
void update_view();
void update_projection();
void update_mvp();

#endif // RENDERING_H_INCLUDED
