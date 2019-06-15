#include "rendering.h"

#include <math.h>
#ifndef M_PI /* ensure that M_PI is defined as it is not required by the standard */
#define M_PI 3.14159265358979323846
#endif

/* Ensure we are using opengl's core profile only */
#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

//#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "dndndefs.h"
#include "shader.h"
#include "matr.h"
#include "texture.h"
#include "input.h"
#include "chunk.h"
#include "player.h"
#include "worldgen.h"
#include "open-simplex-noise.h"
#include "world.h"
#include "entity.h"
#include "longpos.h"

#define ZNEAR 0.1f
#define ZFAR 10000.0f


extern struct longpos player_lpos;
float looked_at[3];
char inreach = 0;


static const GLfloat block_outline_data[] = {
	1.0f, 1.0f, 1.0f,
	1.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 1.0f,

	1.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,

	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 0.0f,

	1.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f,

	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f,

	0.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 0.0f,
};

static const GLfloat crosshair_data[] = {
	-1, -1,
	1, -1,
	1, 1,

	-1, -1,
	1, 1,
	-1, 1
};

static const GLfloat square2D_data[] = {
	-0.5, -0.5,
	0.5, -0.5,
	0.5, 0.5,

	-0.5, -0.5,
	0.5, 0.5,
	-0.5, 0.5
};

static GLfloat model_scale = /*0.50*/1.0f;
static GLfloat model_rotation_angle = 0.0;
static float model_rotation_axis[3] = {1, 0, 0};
static float model_position[3] = {0.0, 0.0, 0.0};
static float camera_center[3] = {0.0, 0.0, 0.0};
static float up_vector[3] = {0.0, 1.0, 0.0};
static float fovDeg = 45.0;

static GLuint VertexArrayID; /* vao */
static GLuint programID; /* shader */
static GLuint programID_gui;
static GLuint programID_outline;
static GLuint mvpID;
static GLuint textureSamplerID;
static GLuint textureSamplerID_gui;
static GLuint screen_dimens_id_gui;
static GLuint scale_id_gui;
static GLuint offset_id_gui;
static GLuint center_id_gui;
static GLuint textureID_crosshair;
static GLuint textureID_hotbar;

static GLuint vertexbuffer_gui;
static GLuint vertexbuffer_outline;

static float mvp[4][4] = {}; /* columns, rows */
static float model[4][4] = {};
static float view[4][4] = {};
static float projection[4][4] = {};


extern chunk* neig[4];

struct mesh
{
	GLuint vertexbuffer;
	GLuint texibuffer;
	Uint32 num_triangles;
};

static struct mesh meshes[(2*CHUNK_LOADING_RANGE-1)*(2*CHUNK_LOADING_RANGE-1)];
static unsigned short meshindices[2*CHUNK_LOADING_RANGE-1][2*CHUNK_LOADING_RANGE-1];
static long meshindices_base_offset[2] = {0, 0}; /*x, z*/


void render_init()
{	
	SDL_Log("#Triangles@world[0][0]: %i", determine_mescha_size(world(0, 0), neig));

	/* This makes our buffer swap syncronized with the monitor's vertical refresh */
	SDL_GL_SetSwapInterval(1);

	for(int x = 0; x < (2*CHUNK_LOADING_RANGE-1); x++) {
		for(int z = 0; z < (2*CHUNK_LOADING_RANGE-1); z++) {
			SDL_Log("generating mesh of chunk [%i][%i]", x, z);
			meshindices[x][z] = x + z * (2*CHUNK_LOADING_RANGE-1);
			//SDL_Log("resulting meshindex: %i", (int) meshindices[x][z]);

			glGenBuffers(1, &meshes[meshindices[x][z]].vertexbuffer);
			glGenBuffers(1, &meshes[meshindices[x][z]].texibuffer);

			SDL_Log("NEIG: %p, %p, %p, %p", neig[0], neig[1], neig[2], neig[3]);
			meshes[meshindices[x][z]].num_triangles = generate_mescha(
					world(x, z),
					neig,
					meshes[meshindices[x][z]].vertexbuffer,
					meshes[meshindices[x][z]].texibuffer
					);
		}
	}

	glGenBuffers(1, &vertexbuffer_gui);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_gui);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair_data), crosshair_data, GL_STATIC_DRAW);


	glGenBuffers(1, &vertexbuffer_outline);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_outline);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(block_outline_data), block_outline_data, GL_STATIC_DRAW);


	textureID_crosshair = texture_from_png("textures/ui/crosshair.png");
	SDL_Log("TX: %u", textureID_crosshair);
	glBindTexture(GL_TEXTURE_2D, textureID_crosshair);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	textureID_hotbar = texture_from_png("textures/ui/hotbar.png");
	glBindTexture(GL_TEXTURE_2D, textureID_hotbar);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	/* start with a background color */
	glClearColor(0.6, 0.8, 1.0, 1.0); /* ungefähr himmelblau*/
	glClear(GL_COLOR_BUFFER_BIT);

	/* create vao */
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, get_textureID());
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);


	programID = LoadShaders("shaders/SimpleVertexShader.vertexshader", "shaders/SimpleFragmentShader.fragmentshader");
	programID_gui = LoadShaders("shaders/GUI_Vertexshader.vertexshader", "shaders/GUI_Fragmentshader.fragmentshader");
	programID_outline = LoadShaders("shaders/Outline_Vertexshader.vertexshader", "shaders/Outline_Fragmentshader.fragmentshader");

	// Get a handle for our "myTextureSampler" uniform
	textureSamplerID = glGetUniformLocation(programID, "my_sampler");
	SDL_Log("sampler: %u", textureSamplerID);

	textureSamplerID_gui = glGetUniformLocation(programID_gui, "my_sampler");
	SDL_Log("sampler_gui: %u", textureSamplerID_gui);
	screen_dimens_id_gui = glGetUniformLocation(programID_gui, "screen_dimens");
	scale_id_gui = glGetUniformLocation(programID_gui, "scale");
	offset_id_gui = glGetUniformLocation(programID_gui, "offset");
	center_id_gui = glGetUniformLocation(programID_gui, "center");

	// Get a handle for our "MVP" uniform
	// Only during the initialisation
	mvpID = glGetUniformLocation(programID, "MVP");

	update_projection();
}

void update_mesh(int x, int z) {
	meshes[meshindices[x][z]].num_triangles = generate_mescha(
			world(x + meshindices_base_offset[0], z + meshindices_base_offset[1]), 
			neig,
			meshes[meshindices[x][z]].vertexbuffer,
			meshes[meshindices[x][z]].texibuffer
			);
}

void update_mesh_abs(long x, long z) {
	//SDL_Log("Updating mesh of chunk absolute: %i|%i, relative: %i|%i", x, z, x-meshindices_base_offset[0], z-meshindices_base_offset[1]);
	meshes[meshindices[x-meshindices_base_offset[0]][z-meshindices_base_offset[1]]].num_triangles = generate_mescha(
			world(x, z), 
			neig,
			meshes[meshindices[x-meshindices_base_offset[0]][z-meshindices_base_offset[1]]].vertexbuffer,
			meshes[meshindices[x-meshindices_base_offset[0]][z-meshindices_base_offset[1]]].texibuffer
			);
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
	//SDL_Log("Model matrix updated:");
	//print_mat4(model);
}

void update_view()
{
	float help0[3] = {};
	get_player_ori(camera_center);
	//SDL_Log("Camera center: %f|%f|%f", camera_center[0], camera_center[1], camera_center[2]); 
	get_player_right(help0);
	//SDL_Log("Player right: %f|%f|%f", help0[0], help0[1], help0[2]);
	//vec3_cross(help0, camera_center, up_vector);

	vec3_add(camera_center, player_lpos.rpos, camera_center);
	//SDL_Log("Player rpos: %f|%f|%f", player_lpos.rpos[0], player_lpos.rpos[1], player_lpos.rpos[2]);

	lookAtRH(player_lpos.rpos, camera_center, up_vector, view); /*now we should have a view matrix*/
	//SDL_Log("View matrix updated:");
	//print_mat4(view);
}

void update_projection()
{
	perspectiveRH((fovDeg * (float)M_PI)/180.0, 4.0/3.0, ZNEAR, ZFAR, projection); /*now we should have a projection matrix*/
}

void render_looper()
{
	int i;
	static long last_player_chunk_offset[2] = {0, 0};

	if((player_lpos.chunk[0] != last_player_chunk_offset[0]) || (player_lpos.chunk[1] != last_player_chunk_offset[1])) {
		/*TODO this should really be done in an asynchronous way to prevent fps drops*/
		/*player changed chunk*/
		long new_meshindices_base_offset[2];
		long offsetchange[2];
		unsigned short new_meshindices[2*CHUNK_LOADING_RANGE-1][2*CHUNK_LOADING_RANGE-1];

		SDL_Log("Changed chunk: player: %f|%f, player_chunk: %ld|%ld, base_offset: %ld|%ld",
				player_lpos.rpos[0], player_lpos.rpos[2], player_lpos.chunk[0], player_lpos.chunk[1],
				meshindices_base_offset[0], meshindices_base_offset[1]);

		new_meshindices_base_offset[0] = player_lpos.chunk[0] - CHUNK_LOADING_RANGE + 1;
		new_meshindices_base_offset[1] = player_lpos.chunk[1] - CHUNK_LOADING_RANGE + 1;

		offsetchange[0] = new_meshindices_base_offset[0] - meshindices_base_offset[0];
		offsetchange[1] = new_meshindices_base_offset[1] - meshindices_base_offset[1];

		for(int x = 0; x < (2*CHUNK_LOADING_RANGE-1); x++) {
			for(int z = 0; z < (2*CHUNK_LOADING_RANGE-1); z++) {
				if(	((x - offsetchange[0]) >= 0) && ((z - offsetchange[1]) >= 0) &&
						(((x - offsetchange[0]) < (2*CHUNK_LOADING_RANGE-1))) && (((z - offsetchange[1]) < (2*CHUNK_LOADING_RANGE-1)))) {
					/*keep that mesh but remap it*/
					new_meshindices[x-offsetchange[0]][z-offsetchange[1]] = meshindices[x][z];
				} else {
					/*make new mesh*/
					/*invert old distance from old center to get new distance from new center*/
					new_meshindices[2*(CHUNK_LOADING_RANGE - 1) - x][2*(CHUNK_LOADING_RANGE - 1) - z] = meshindices[x][z];
					//SDL_Log("Meshindex: %i - [%i][%i]", (int) meshindices[x][z], x, z);
					meshes[meshindices[x][z]].num_triangles = generate_mescha(
							world(	2*(CHUNK_LOADING_RANGE - 1) - x + new_meshindices_base_offset[0],
								2*(CHUNK_LOADING_RANGE - 1) - z + new_meshindices_base_offset[1]), 
							neig,
							meshes[meshindices[x][z]].vertexbuffer,
							meshes[meshindices[x][z]].texibuffer
							);
					SDL_Log(	"making new mesh at %ld|%ld into %i",
							2*(CHUNK_LOADING_RANGE - 1) - x + new_meshindices_base_offset[0],
							2*(CHUNK_LOADING_RANGE - 1) - z + new_meshindices_base_offset[1],
							meshindices[x][z]);
					//SDL_Log("CHUNK[%i][%i] - [%i][%i]", 2*(CHUNK_LOADING_RANGE - 1) - x + new_meshindices_base_offset[0], 2*(CHUNK_LOADING_RANGE - 1) - z + new_meshindices_base_offset[1], x, z);
				}
			}
		}

		for(int x = 0; x < (2*CHUNK_LOADING_RANGE-1); x++) {
			for(int z = 0; z < (2*CHUNK_LOADING_RANGE-1); z++) {
				SDL_Log("writing meshindex: %i - [%i][%i]", (int) new_meshindices[x][z], x, z);
				meshindices[x][z] = new_meshindices[x][z];
			}
		}

		meshindices_base_offset[0] = new_meshindices_base_offset[0];
		meshindices_base_offset[1] = new_meshindices_base_offset[1];

		last_player_chunk_offset[0] = player_lpos.chunk[0];
		last_player_chunk_offset[1] = player_lpos.chunk[1];
	}

	glUseProgram(programID);

	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	/*block/terrain rendering*/
	update_view();

	for(int x = 0; x < (2*CHUNK_LOADING_RANGE-1); x++) {
		for(int z = 0; z < (2*CHUNK_LOADING_RANGE-1); z++) {
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, meshes[meshindices[x][z]].vertexbuffer);
			glVertexAttribPointer(
					0,                  // attribute 0. Must match the layout in the shader.
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
					);


			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, meshes[meshindices[x][z]].texibuffer);
			glVertexAttribPointer(
					2,                  
					1, 
					GL_UNSIGNED_INT,
					GL_FALSE,
					0, 
					(void*)0 
					);
			model_position[0] = CHUNK_LIM_HOR * model_scale * 
				(x + meshindices_base_offset[0] - player_lpos.chunk[0]);
			model_position[1] = 0;
			model_position[2] = CHUNK_LIM_HOR * model_scale *
				(z + meshindices_base_offset[1] - player_lpos.chunk[1]);
			//SDL_Log("X: %f, Z: %f", model_position[0], model_position[2]);

			update_model();
			update_mvp();

			//single texture
			glUniform1i(textureSamplerID, 0);
			glDrawArrays(GL_TRIANGLES, 0, 3*meshes[meshindices[x][z]].num_triangles);
			
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(2);
		}
	}

	/*block picking*/
	if(inreach = pick_block(looked_at)) {
		/*selected block outline*/
		glUseProgram(programID_outline);
		glDisable(GL_DEPTH_TEST);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_outline);
		glVertexAttribPointer(
				0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
				);

		memcpy(model_position, looked_at, 3 * sizeof(float));

		//SDL_Log("Postion for outline updated: %f|%f|%f", model_position[0], model_position[1], model_position[2]); 

		update_model();
		update_mvp();

		glLineWidth(2.0f);

		glDrawArrays(GL_LINE_LOOP, 0, 4);
		glDrawArrays(GL_LINE_LOOP, 4, 4);
		glDrawArrays(GL_LINES, 8, 8);

		glDisableVertexAttribArray(0);
	}
	
	/* entity rendering */
	render_entities(view, projection);


	/*gui rendering*/
	glUseProgram(programID_gui);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_gui);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(textureSamplerID_gui, 0);
	glUniform2f(screen_dimens_id_gui, 640.0f, 480.0f);
	glUniform2f(scale_id_gui, 24.0f, 24.0f);
	glUniform2f(offset_id_gui, 0.0f, 0.0f);
	glUniform2f(center_id_gui, 0.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, textureID_crosshair);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	/*hotbar*/
	glUniform1i(textureSamplerID_gui, 0);
	glUniform2f(screen_dimens_id_gui, 640.0f, 480.0f);
	glUniform2f(scale_id_gui, 6*81.0f, 6*10.0f);
	glUniform2f(offset_id_gui, 0.0f, -1.0f);
	glUniform2f(center_id_gui, 0.0f, -1.0f);
	glBindTexture(GL_TEXTURE_2D, textureID_hotbar);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);
}

int pick_block(float *rrpos)
{
	GLfloat depth;
	float zLinear;
	float mhelp0[4][4] = {}, mhelp1[4][4] = {};
	float vcenter[4] = {0.0f, 0.0f, 0.0f, 1.0f}, vresult[4] = {};
	struct longpos lpos;
	unsigned int block[3];
	glReadPixels(320,  240,  1,  1,  GL_DEPTH_COMPONENT,  GL_FLOAT,  &depth);
	//SDL_Log("Depth: %f", depth);
	vcenter[2] = 2.0f * depth - 1.0f;
	zLinear = 2.0 * ZNEAR * ZFAR / (ZFAR + ZNEAR - vcenter[2] * (ZFAR - ZNEAR));
	//SDL_Log("Dist: %f", zLinear);
	if(zLinear > PLAYER_REACH) {
		return 0;
	}
	mult_mat4_mat4(projection, view, mhelp0);
	inv_mat4(mhelp0, mhelp1);
	//print_mat4(mhelp1);
	mult_mat4_vec4(mhelp1, vcenter, vresult);
	vresult[0] *= 1.0f / vresult[3];
	vresult[1] *= 1.0f / vresult[3];
	vresult[2] *= 1.0f / vresult[3];
	vresult[3] = 1.0f;

	rrpos_to_lpos(vresult, player_lpos, lpos); /* this is necessary in order to access the actual world data */
	//SDL_Log("in chunk: %i|%i", cx, cz);

	if(world(lpos.chunk[0], lpos.chunk[1])->data[(int)floor(lpos.rpos[0])][(int)lpos.rpos[1]][(int)floor(lpos.rpos[2])].id == 0) {
		//find the side of the block we appear to be looking at
		GLfloat over[3];
		char inversion[3] = {0, 0, 0};
		GLfloat vsmallest;
		char nsmallest = 0;
		for(int n = 0; n < 3; n++) {
			over[n] = vresult[n] - (int) floor(vresult[n]);
			if(over[n] > 0.5f) {
				inversion[n] = 1;
				over[n] = 1.0f - over[n];
			}
		}
		vsmallest = over[0];
		for(int n = 1; n < 3; n++) {
			if(over[n] < vsmallest) {
				vsmallest = over[n];
				nsmallest = n;
			}
		}
		vresult[nsmallest] = ((int)floor(vresult[nsmallest])) + (2.0f * inversion[nsmallest] - 1.0f);
	}
	vresult[0] = (int)floor(vresult[0]);
	vresult[1] = (int)vresult[1];
	vresult[2] = (int)floor(vresult[2]);

	memcpy(rrpos, vresult, 3 * sizeof(float));

	return 1;
}

