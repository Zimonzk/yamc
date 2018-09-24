#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

#include <GL/gl.h>

// Load a .BMP file using our custom loader
void loadBMP_custom(const char * imagepath, GLuint textureID, unsigned int blockid);

GLuint load_bmp_gui(const char* imagepath);

void dumpBMPdat(const char *data, size_t imageSize);

#endif // TEXTURE_H_INCLUDED
