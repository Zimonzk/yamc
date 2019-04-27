#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GL3_PROTOTYPES 1
#if defined(__APPLE__)
#include <OpenGL/glew.h>
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#include <SDL2/SDL.h>
#include "SOIL.h"

static GLuint textureID;

unsigned int load_block_texture(const char *imagepath)
{
	static int first = 1;
	static unsigned int texi = 0;
	int width, height, channels;
	unsigned char* imagedata = SOIL_load_image(imagepath, &width, &height, &channels, SOIL_LOAD_RGBA);

	/* invert image Y */
	for(int j = 0; j*2 < height; ++j) {
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;
		for(int i = width * channels; i > 0; --i) {
			unsigned char temp = imagedata[index1];
			imagedata[index1] = imagedata[index2];
			imagedata[index2] = temp;
			index1++;
			index2++;
		}
	}

	if(first) {
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width, height, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		first = 0;
	} else {
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
	}
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, texi, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, imagedata);

	// OpenGL has now copied the data. Free our own version
	free(imagedata);

	// Poor filtering
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	SDL_Log("Finished loading texture");
	return texi++;
}

GLuint get_textureID() {
	/* TODO we might have multiple textureIDs later since there is a limit to the ammount of layers in a 2D texture array
	 * that will require quite some code restructuring */
	return textureID;
}
