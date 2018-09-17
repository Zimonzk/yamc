#ifndef RENDERING_H_INCLUDED
#define RENDERING_H_INCLUDED

#include "chunk.h"

void render_init();
void render();
void render_chunk(chunk* cchunk);

void update_model();
void update_view();
void update_projection();
void update_mvp();

#endif // RENDERING_H_INCLUDED
