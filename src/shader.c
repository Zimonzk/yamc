#include "zio-list.h"
#include "zio-reader.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint result, infologlength;

    FILE *vsf, *fsf;

    int c;

    kostring vss,fss;
    kostring_zero(&vss);
    kostring_zero(&fss);
    vss.block_bytes = 1024;
    fss.block_bytes = 1024;

    /* read the vertexshader*/
    vsf = fopen(vertex_file_path, "r");

    if(vsf == NULL) {
        SDL_Log("No Vertexshader, exiting!");
        exit(1);
    }

    c = getc(vsf);
    while(c != EOF) {
        kostring_append(&vss, c);
        c = getc(vsf);
    }

	// Compile Vertex Shader
	SDL_Log("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = vss.cstring;
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &infologlength);
	if ( infologlength > 0 ){
		kostring infolog;
		kostring_zero(&infolog);
		infolog.block_bytes = 128;
		kostring_resize(&infolog, infologlength+1);
		glGetShaderInfoLog(VertexShaderID, infologlength, NULL, infolog.cstring);
		SDL_Log("%s\n", infolog.cstring);
		kostring_free(&infolog);
	}

    /* read the fragmentshader */
    fsf = fopen(fragment_file_path, "r");

    if(fsf == NULL) {
        SDL_Log("No Fragmentshader, exiting!");
        exit(1);
    }

    c = getc(fsf);
    while(c != EOF) {
        kostring_append(&fss, c);
        c = getc(fsf);
    }

	// Compile Fragment Shader
	SDL_Log("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = fss.cstring;
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Vertex Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &infologlength);
	if ( infologlength > 0 ){
		kostring infolog;
		kostring_zero(&infolog);
		infolog.block_bytes = 128;
		kostring_resize(&infolog, infologlength+1);
		glGetShaderInfoLog(FragmentShaderID, infologlength, NULL, infolog.cstring);
		SDL_Log("%s\n", infolog.cstring);
		kostring_free(&infolog);
	}


	// Link the program
	SDL_Log("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &infologlength);
	if ( infologlength > 0 ){
        kostring infolog;
        kostring_zero(&infolog);
		infolog.block_bytes = 128;
		kostring_resize(&infolog, infologlength+1);
		glGetProgramInfoLog(ProgramID, infologlength, NULL, infolog.cstring);
		SDL_Log("%s\n", infolog.cstring);
		kostring_free(&infolog);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

    kostring_free(&vss);
    kostring_free(&fss);

	return ProgramID;
}

