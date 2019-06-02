#include <stdio.h>

/*for exporting the altas al test*/
#include <png.h>

/* Ensure we are using opengl's core profile only */
#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif


#include "fonter.h"
#include "zimonzk/lists.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

static GLuint textureID_font;
static GLuint vb_text;
static GLuint programID_font;

static GLuint textureSamplerID_text;
static GLuint uniformID_textcolor;
static GLuint uniformID_screen_dimens;
static GLuint uniformid_start;

int initfont()
{
	FILE *fontfile = 0;
	arraylist fontdata;
	unsigned char font_bitmap[512*512];
	stbtt_bakedchar cdata[1024];
	int c;

	arraylist_init(&fontdata, 1, 1024);

	fontfile = fopen("fonts/OpenSans-Regular.ttf", "rb");
	if(fontfile == 0) {
		return -1;
	}

	while(1) {
		c = fgetc(fontfile);
		if(c == EOF) {
			break;
		}
		arraylist_append(&fontdata, (unsigned char *)&c);
	}

	if(stbtt_BakeFontBitmap(fontdata.data, 0, 24.0, font_bitmap, 512, 512, 32, 1024, cdata) < 0) {
		return -2;
	}

	arraylist_delete(&fontdata);

	/* put bitmap into opengl texture */
	glGenTextures(1, &textureID_font);
	glBindTexture(GL_TEXTURE_2D, textureID_font);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font_bitmap);
	// can free temp_bitmap at this point
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenBuffers(1, &vb_text);

	programID_font = LoadShaders("shaders/text.vertexshader", "shaders/text.fragmentshader");
	textureSamplerID_text = glGetUniformLocation(programID_font, "my_sampler");
	uniformID_textcolor = glGetUniformLocation(programID_font, "textcolor");
	uniformID_screen_dimens = glGetUniformLocation(programID_font, "screen_dimens");
	uniformID_start = glGetUniformLocation(programID_font, "start");



	{ /* png export fpr testing */
		FILE *fp = fopen("fonts/atlasexport.png", "wb");
		png_structp png_ptr = NULL;
		png_infop info_ptr = NULL;
		size_t x, y;
		png_uint_32 bytes_per_row;
		png_byte **row_pointers = NULL;

		if (fp == NULL) return -1;

		/* Initialize the write struct. */
		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png_ptr == NULL) {
			fclose(fp);
			return -1;
		}

		/* Initialize the info struct. */
		info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL) {
			png_destroy_write_struct(&png_ptr, NULL);
			fclose(fp);
			return -1;
		}

		/* Set up error handling. */
		if (setjmp(png_jmpbuf(png_ptr))) {
			png_destroy_write_struct(&png_ptr, &info_ptr);
			fclose(fp);
			return -1;
		}

		/* Set image attributes. */
		png_set_IHDR(png_ptr,
				info_ptr,
				512,
				512,
				8,
				PNG_COLOR_TYPE_GRAY,
				PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT);

		/* Initialize rows of PNG. */
		row_pointers = malloc(sizeof(unsigned char *) * 512);
		for(int row = 0; row < 512; row++) {
			row_pointers[row] = &font_bitmap[512 * row];
		}

		/* Actually write the image data. */
		png_init_io(png_ptr, fp);
		png_set_rows(png_ptr, info_ptr, row_pointers);
		png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

		png_free(png_ptr, row_pointers);

		/* Finish writing. */
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
	}

	return 0;
}

void render_text(char *utf8str, float x, float y) /* x, y openGL coordinates 
						   * of where the first line starts. */
{
	glEnablaVertexAttribArray(0); /* this enables the vertex coordinates 
				       * (location = 0) */
	glVertexAttribPointer( /* this sets the settings for the vertex coordinates */
			0,                  // attribute 0. Must match the layout in the shader.
			2,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer(GL_ARRAY_BUFFER, vb_text);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID_text);

	glUniform1i(textureSamplerID_text, 0);
	glUniform2f(uniformID_start, x, y);
	glUniform4f(uniformID_textcolor, 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform2f(uniformID_screen_dimens, 640.0f, 480.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glDisable(GL_DEPTH_TEST);

	for(c = *utfstr; 
}
