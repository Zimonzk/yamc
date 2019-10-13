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

#include <SDL2/SDL.h>


#include "fonter.h"
#include "zimonzk/lists.h"
#include "texture.h"
#include "shader.h"
#include "settings.h"


#define wheight gamesettings.videosettings.height
#define wwidth gamesettings.videosettings.width

static GLuint textureID_font;
static GLuint vb_text;
static GLuint programID_font;

static GLuint textureSamplerID_text;
static GLuint uniformID_textcolor;
static GLuint uniformID_size;
static GLuint uniformID_start;

int initfont()
{
	unsigned char* font_bitmap;
	int width, height;

	/*png read*/
	font_bitmap = grayscale_from_png("fonts/Latin-1-atlas.png"/*"fonts/TEST.png"*/, &width, &height);
	if(font_bitmap == 0) {
		SDL_Log("Error: Could not load font atlas.");
		return -1;
	}

	if((width != height) || (width % 16 != 0)) {
		SDL_Log("Error: Texture atlas has wrong format. Please provide an image with the same height as width which are a multiple of 16.");
		return -2;
	}
	//SDL_Log("DDDD %i %i", width, height);

	/* put bitmap into opengl texture */
	glGenTextures(1, &textureID_font);
	glBindTexture(GL_TEXTURE_2D, textureID_font);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, font_bitmap);	
	free(font_bitmap);

	//textureID_font = texture_from_png("textures/blocks/stone.png");
	//glBindTexture(GL_TEXTURE_2D, textureID_font);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glGenBuffers(1, &vb_text);

	programID_font = LoadShaders("shaders/Text.vertexshader", "shaders/Text.fragmentshader");
	if(programID_font == 0) {
		SDL_Log("Problem loading text shaders.");
		return -3;
	}
	textureSamplerID_text = glGetUniformLocation(programID_font, "my_sampler");
	uniformID_textcolor = glGetUniformLocation(programID_font, "textcolor");
	uniformID_size = glGetUniformLocation(programID_font, "size");
	uniformID_start = glGetUniformLocation(programID_font, "start");

	return 0;
}

void render_text(char *str, float x, float y) /* x, y openGL coordinates 
						   * of where the first line starts. */
{
	arraylist uv_temp;
	arraylist_init(&uv_temp, sizeof(float), 2);
	//SDL_Log("000");
	//SDL_Log("%i", glGetError());
	
	glUseProgram(programID_font);
	//SDL_Log("prog: %i", programID_font);
	//SDL_Log("%i", glGetError());

	glEnableVertexAttribArray(0); /* this enables the vertex coordinates 
				       * (location = 0) */
	//SDL_Log("%i", glGetError());

	glActiveTexture(GL_TEXTURE0);
	//SDL_Log("%i", glGetError());

	glBindTexture(GL_TEXTURE_2D, textureID_font);
	//SDL_Log("%i", glGetError());

	glUniform1i(textureSamplerID_text, 0);
	//SDL_Log("%i", glGetError());

	glUniform2f(uniformID_start, x, y);
	//SDL_Log("%i", glGetError());

	glUniform4f(uniformID_textcolor, 1.0f, 1.0f, 1.0f, 1.0f);
	//SDL_Log("%i", glGetError());

	glUniform2f(uniformID_size, 32.0f/wwidth, 32.0f/wheight);
	//SDL_Log("%i", glGetError());


	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	//SDL_Log("%i, %i", GL_INVALID_OPERATION, GL_INVALID_VALUE);
	//SDL_Log("0-%i", glGetError());


	for(char c = *str; c != '\0'; c = *(++str)) {
		float u, v;
		//SDL_Log("%c", c);
		u = c % 16;
		v = c / 16;

		u = u / 16.0f;
		v = 1.0f - (v / 16.0f);

		v -= 1.0f/16.0f;

		arraylist_append(&uv_temp, &u);
		arraylist_append(&uv_temp, &v);

		u += 1.0f/16.0f;

		arraylist_append(&uv_temp, &u);
		arraylist_append(&uv_temp, &v);

		v += 1.0f/16.0f;

		arraylist_append(&uv_temp, &u);
		arraylist_append(&uv_temp, &v);

		u -= 1.0f/16.0f;
		v -= 1.0f/16.0f;

		arraylist_append(&uv_temp, &u);
		arraylist_append(&uv_temp, &v);

		u += 1.0f/16.0f;
		v += 1.0f/16.0f;

		arraylist_append(&uv_temp, &u);
		arraylist_append(&uv_temp, &v);

		u -= 1.0f/16.0f;

		arraylist_append(&uv_temp, &u);
		arraylist_append(&uv_temp, &v);
	}


	glBindBuffer(GL_ARRAY_BUFFER, vb_text);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * uv_temp.used_units, uv_temp.data, GL_STREAM_DRAW);
	//SDL_Log("%i", uv_temp.used_units);
	
	glVertexAttribPointer( /* this sets the settings for the vertex coordinates */
		0,                  // attribute 0. Must match the layout in the shader.
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);
	
	//glVertexAttribDivisor(0, 6);

	glDrawArrays(GL_TRIANGLES, 0, uv_temp.used_units / 2);

	//SDL_Log("1-%i", glGetError());
	
	arraylist_delete(&uv_temp);

	//glVertexAttribDivisor(0, 0);
	glEnable(GL_DEPTH_TEST);
}
