#include <zimonzk/lists.h>

#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <SDL2/SDL.h>

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint result, infologlength;

	FILE *vsf, *fsf;

	int c;

	arraylist vss,fss;
	arraylist_init(&vss, 1, 1024);
	arraylist_init(&fss, 1, 1024);

	/* read the vertexshader*/
	vsf = fopen(vertex_file_path, "r");

	if(vsf == NULL) {
		SDL_Log("No Vertexshader, exiting!");
		exit(1);
	}

	c = getc(vsf);
	while(c != EOF) {
		arraylist_append(&vss, &c);
		c = getc(vsf);
	}

	// Compile Vertex Shader
	SDL_Log("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = vss.data;
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &infologlength);
	if ( infologlength > 0 ){
		char *infolog = calloc(infologlength+1, 1);
		glGetShaderInfoLog(VertexShaderID, infologlength, NULL, infolog);
		SDL_Log("%s\n", infolog);
		free(infolog);
	}

	/* read the fragmentshader */
	fsf = fopen(fragment_file_path, "r");

	if(fsf == NULL) {
		SDL_Log("No Fragmentshader, exiting!");
		exit(1);
	}

	c = getc(fsf);
	while(c != EOF) {
		arraylist_append(&fss, &c);
		c = getc(fsf);
	}

	// Compile Fragment Shader
	SDL_Log("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = fss.data;
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Vertex Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &infologlength);
	if ( infologlength > 0 ){
		char *infolog = calloc(infologlength+1, 1);
		glGetShaderInfoLog(FragmentShaderID, infologlength, NULL, infolog);
		SDL_Log("%s\n", infolog);
		free(infolog);
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
		char *infolog = calloc(infologlength+1, 1);
		glGetShaderInfoLog(ProgramID, infologlength, NULL, infolog);
		SDL_Log("%s\n", infolog);
		free(infolog);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	arraylist_delete(&vss);
	arraylist_delete(&fss);

	return ProgramID;
}

