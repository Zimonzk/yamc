#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <SDL2/SDL.h>
#include <png.h>

static GLuint textureID;

unsigned int load_block_texture(const char *imagepath)
{
	static int first = 1;
	static unsigned int texi = 0;
	int width, height, channels = 4;
	unsigned char* imagedata = image_from_png(imagepath, &width, &height);


	if(first) {
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width, height, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		first = 0;
	} else {
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
	}
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, texi, width, height, 1, channels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, imagedata);

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



/* make an RGBA 8 bit per color bitmap from a png file
 * the returned buffer must be freed by the caller */
unsigned char *image_from_png(const char *png_path, int *width, int *height)
{
	int channels, bit_depth, color_type;
	unsigned char* imagedata;
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned char **rows = 0;

	
	fp = fopen(png_path, "rb");
	if(fp == NULL) {
		return 0;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (png_ptr == NULL) {
		fclose(fp);
		return 0;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return 0;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* Free all of the memory associated with the png_ptr and info_ptr. */
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return 0;
	}

	png_init_io(png_ptr, fp);

	png_read_info(png_ptr, info_ptr);

	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	channels = png_get_channels(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	*width = png_get_image_width(png_ptr, info_ptr);
	*height = png_get_image_height(png_ptr, info_ptr);	


	// Read any color_type into 8bit depth, RGBA format.
	// See http://www.libpng.org/pub/png/libpng-manual.txt

	if(bit_depth == 16)
		png_set_strip_16(png_ptr);

	if(color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png_ptr);

	if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	// These color_type don't have an alpha channel then fill it with 0xff.
	if(color_type == PNG_COLOR_TYPE_RGB ||
			color_type == PNG_COLOR_TYPE_GRAY ||
			color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);

	if(color_type == PNG_COLOR_TYPE_GRAY ||
			color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	channels = png_get_channels(png_ptr, info_ptr);
	SDL_Log("W%iH%iC%iD%i", *width, *height, channels, bit_depth);

	imagedata = calloc(*width * *height * channels, 1);
	rows = calloc(*height, sizeof(char *));
	for(int i = 0; i < *height; i++) {
		rows[i] = imagedata + (*height - 1 - i) * *width * channels;
	}

	png_read_image(png_ptr, rows);

	/* Clean up after the read, and free any memory allocated.  REQUIRED. */
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
	
	return imagedata;
}

/* takes an RGBA 8 bit per color bitmap and
 * puts its content into an OpenGL texture */
GLuint texture_from_image(char *image, int width, int height)
{
	GLuint textureID;

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	return textureID;
}

/* makes an OpenGL texture from a png file */
GLuint texture_from_png(char *png_path)
{
	GLuint textureID;
	int width, height;
	unsigned char *imagedata = 0;

	imagedata = image_from_png(png_path, &width, &height);
	if(imagedata == 0) {
		return 0;
	}

	textureID = texture_from_image(imagedata, width, height);

	free(imagedata);
	return textureID;
}


