#include "rendering.h"

#include <math.h>
/* If using gl3.h */
/* Ensure we are using opengl's core profile only */
#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/gl.h>

//#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "dndndefs.h"
#include "shader.h"
#include "matr.h"
#include "texture.h"
#include "input.h"
#include "chunk.h"
#include "player.h"

#define NUMVERT 36

extern long player_chunk_offset[2];

// An array of 3 vectors which represents 3 vertices
static const GLfloat g_vertex_buffer_data[] = {
	-1, -1, 1, /* cube front lower */
	1, -1, 1,
	1, 1, 1,

	-1, -1, 1, /* cube front upper */
	1, 1, 1,
	-1, 1, 1,

	1, -1, 1, /* cube right lower */
	1, -1, -1,
	1, 1, -1,

	1, -1, 1, /* cube right upper */
	1, 1, -1,
	1, 1, 1,

	1, -1, -1, /* cube back lower */
	-1, -1, -1,
	-1, 1, -1,

	1, -1, -1, /* cube back upper */
	-1, 1, -1,
	1, 1, -1,

	-1, -1, -1, /* cube left lower */
	-1, -1, 1,
	-1, 1, 1,

	-1, -1, -1, /* cube left upper */
	-1, 1, 1,
	-1, 1, -1,

	-1, 1, 1, /* cube top front */
	1, 1, 1,
	1, 1, -1,

	-1, 1, 1, /* cube top back */
	1, 1, -1,
	-1, 1, -1,

	1, -1, 1, /* cube bottom front */
	-1, -1, 1,
	-1, -1, -1,

	1, -1, 1, /* cube bottom back */
	-1, -1, -1,
	1, -1, -1,
};

static const GLfloat g_uv_buffer_data[] = {
	0, 0,
	1, 0,
	1, 1,

	0, 0,
	1, 1,
	0, 1,


	0, 0,
	1, 0,
	1, 1,

	0, 0,
	1, 1,
	0, 1,


	0, 0,
	1, 0,
	1, 1,

	0, 0,
	1, 1,
	0, 1,


	0, 0,
	1, 0,
	1, 1,

	0, 0,
	1, 1,
	0, 1,


	0, 0,
	1, 0,
	1, 1,

	0, 0,
	1, 1,
	0, 1,


	0, 0,
	1, 0,
	1, 1,

	0, 0,
	1, 1,
	0, 1
};

static GLfloat model_scale = 0.50;
static GLfloat model_rotation_angle = 0.0;
static float model_rotation_axis[3] = {1, 0, 0};
static float model_position[3] = {0.0, 0.0, 0.0};
static float camera_position[3] = {4.0, 3.0, 3.0};
static float camera_center[3] = {0.0, 0.0, 0.0};
static float up_vector[3] = {0.0, 1.0, 0.0};
static float fovDeg = 45.0;

static GLuint VertexArrayID; /* vao */
static GLuint programID; /* shader */
static GLuint mvpID;
static GLuint texture;
static GLuint textureSamplerID;
static GLuint texture2;
// This will identify our vertex buffer
static GLuint vertexbuffer;
static GLuint uvbuffer;

static float mvp[4][4] = {}; /* columns, rows */
static float model[4][4] = {};
static float view[4][4] = {};
static float projection[4][4] = {};

static Uint32 numTriangles;

static GLuint texibuffer;

extern chunk world[(2*CHUNK_LOADING_RANGE)+1][(2*CHUNK_LOADING_RANGE)+1];
extern chunk* neig[4];


void render_init()
{
	/* init glew */
	glewExperimental = GL_TRUE;
	glewInit();

	/* This makes our buffer swap syncronized with the monitor's vertical refresh */
	SDL_GL_SetSwapInterval(0);


	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);


	glGenBuffers(1, &texibuffer);
	numTriangles = generate_mescha(&world[0][0], neig, vertexbuffer, texibuffer);


	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// Create one OpenGL texture
	/*texture = */loadBMP_custom("textures/blocks/side.bmp", textureID, 1);
	/*texture2 = */loadBMP_custom("textures/blocks/top.bmp", textureID, 2);


	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	/* start with a background color */
	glClearColor( 0.6, 0.8, 1.0, 1.0 ); /* ungefähr himmelblau*/
	glClear ( GL_COLOR_BUFFER_BIT );

	/* create vao */
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

	/*glActiveTexture(GL_TEXTURE1);
	  glBindTexture(GL_TEXTURE_2D, texture2);*/


	programID = LoadShaders("shaders/SimpleVertexShader.vertexshader", "shaders/SimpleFragmentShader.fragmentshader");
	glUseProgram(programID);

	// Get a handle for our "myTextureSampler" uniform
	textureSamplerID = glGetUniformLocation(programID, "my_sampler");
	SDL_Log("sampler: %i", textureSamplerID);

	// Get a handle for our "MVP" uniform
	// Only during the initialisation
	mvpID = glGetUniformLocation(programID, "MVP");

	update_projection();
}

void update_mvp()
{
	float help0[4][4] = {};
	mult_mat4_mat4(view, model, help0);
	mult_mat4_mat4(projection, help0, mvp); /*now we should have a real mvp matrix*/

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
	glUniformMatrix4fv(mvpID, 1, GL_FALSE, mvp);
}

void update_model()
{
	get_player_pos(camera_position);
	float model_translation_mat[4][4] = {
		{1,0,0,0},
		{0,1,0,0},
		{0,0,1,0},
		{model_position[0],model_position[1],model_position[2],1}};
	float model_scaling_matrix[4][4] = {
		{model_scale,0,0,0},
		{0,model_scale,0,0},
		{0,0,model_scale,0},
		{0,0,0,1}};
	/*float mrq[4] = { /* model rotation quaternion *//*
							     model_rotation_axis[0] * (float) sin(model_rotation_angle / 2),
							     model_rotation_axis[1] * sin(model_rotation_angle / 2),
							     model_rotation_axis[2] * sin(model_rotation_angle / 2),
							     cos(model_rotation_angle / 2)};
							     float mehl1[4][4] = {
							     {mrq[3], -mrq[2], mrq[1], -mrq[0]},
							     {mrq[2], mrq[3], -mrq[0], -mrq[1]},
							     {-mrq[1], mrq[0], mrq[3], -mrq[2]},
							     {mrq[0], mrq[1], mrq[2], mrq[3]}};
							     float mehl2[4][4] = {
							     {mrq[3], -mrq[2], mrq[1], mrq[0]},
							     {mrq[2], mrq[3], -mrq[0], mrq[1]},
							     {-mrq[1], mrq[0], mrq[3], mrq[2]},
							     {-mrq[0], -mrq[1], -mrq[2], mrq[3]}};
							     float model_rotation_mat[4][4] = {};
							     float help0[4][4] = {};

							     mult_mat4_mat4(mehl1, mehl2, model_rotation_mat);

							     mult_mat4_mat4(model_rotation_mat, model_scaling_matrix, help0);

							     mult_mat4_mat4(model_translation_mat, help0, model); /*now we should have a model matrix*/
	mult_mat4_mat4(model_translation_mat, model_scaling_matrix, model);
}

void update_view()
{
	float help0[4][4] = {};
	get_player_ori(camera_center);
	get_player_right(help0);
	vec3_cross(help0, camera_center, up_vector);

	camera_position[0] = 0;
	camera_position[1] = 0;
	camera_position[2] = 0;

	vec3_add(camera_center, camera_position, camera_center);

	lookAtRH(camera_position, camera_center, up_vector, view); /*now we should have a view matrix*/
}

void update_projection()
{
	perspectiveRH((fovDeg * (float)M_PI)/180.0, 4.0/3.0, 0.1, 100.0, projection); /*now we should have a projection matrix*/
}

void render()
{

}

void render_chunk(chunk* cchunk)
{
	//TODO
	int x = 0, y = 0, z = 0;
	int i;
	float playerpos[3];

	get_player_pos(playerpos);

	update_view();

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	// 2nd attribute buffer : UVs
	//glEnableVertexAttribArray(1);
	//glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	/*glVertexAttribPointer(
	  1,                 // attribute. No particular reason for 1, but must match the layout in the shader.
	  2,                 // size : U+V => 2
	  GL_FLOAT,          // type
	  GL_FALSE,          // normalized?
	  0,                 // stride
	  (void*)0           // array buffer offset
	  );*/

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, texibuffer);
	glVertexAttribPointer(
			2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			1,                  // size
			GL_UNSIGNED_INT,    // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*for(x=0; x<1; x++) {
	  for(z=0; z<1; z++) {
	  for(y=0; y<1; y++) {
	  model_position[0] = x - playerpos[0] + 64 * (cchunk->offset_X - player_chunk_offset[0]);
	  model_position[1] = y - playerpos[1];
	  model_position[2] = z - playerpos[2] + 64 * (cchunk->offset_Z - player_chunk_offset[1]);

	  update_model();
	  update_mvp();

	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(textureID, 0);
	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, NUMVERT-12); // Starting from vertex 0; 3 vertices total -> 1 triangle

	glUniform1i(textureID, 1);
	glDrawArrays(GL_TRIANGLES, NUMVERT-12, 12);
	}
	}
	}*/
	model_position[0] = x - playerpos[0] + 64 * (cchunk->offset_X - player_chunk_offset[0]);
	model_position[1] = y - playerpos[1];
	model_position[2] = z - playerpos[2] + 64 * (cchunk->offset_Z - player_chunk_offset[1]);

	update_model();
	update_mvp();

	/*too slow
	  for(i=0; i < numTriangles; i+=4) {
	  glUniform1i(textureID, 0);
	  glDrawArrays(GL_TRIANGLES, 3*i, 6);
	  glUniform1i(textureID, 1);
	  glDrawArrays(GL_TRIANGLES, 3*(i+2), 6);
	  }*/
	//single texture
	glUniform1i(textureSamplerID, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3*numTriangles);

	glDisableVertexAttribArray(0);
	//glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}
