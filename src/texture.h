#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

/* loads a texture for use in block texturing
 * returns an id which can be used as texture id for any block-side */
unsigned int load_block_texture(const char *imagepath);

GLuint get_textureID();

/* make an RGBA 8 bit per color bitmap from a png file
 * the returned buffer must be freed by the caller */
unsigned char *image_from_png(const char *png_path, int *width, int *height);

/* takes an RGBA 8 bit per color bitmap and
 * puts its content into an OpenGL texture */
GLuint texture_from_image(char *image, int width, int height);

/* makes an OpenGL texture from a png file */
GLuint texture_from_png(char *png_path);

#endif // TEXTURE_H_INCLUDED
