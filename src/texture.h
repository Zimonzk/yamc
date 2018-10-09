#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

#include <GL/gl.h>

/* loads a texture for use in block texturing
 * returns an id which can be used as texture id for any block-side */
unsigned int load_block_texture(const char *imagepath);

GLuint get_textureID();

#endif // TEXTURE_H_INCLUDED
