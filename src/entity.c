#include "entity.h"

#include "matr.h"
#include "shader.h"

#include <string.h>

#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <SDL2/SDL.h>

#include "zimonzk/lists.h"

static void sdldie(const char *msg)
{
	SDL_Log("%s: %s\n", msg, SDL_GetError());
	SDL_Quit();
	exit(1);
}


static const GLfloat square2Din3D_data[] = {
	-0.5, 0.0, 0.0,
	0.5, 0.0, 0.0, 
	0.5, 1.0, 0.0,

	-0.5, 0.0, 0.0,
	0.5, 1.0, 0.0,
	-0.5, 1.0, 0.0
};

static GLuint vertexbuffer_billboard;

static GLuint programID_billboard;
static GLuint billboard_uniformID_scale;
static GLuint billboard_uniformID_position;
static GLuint billboard_uniformID_CameraRight_worldspace;
static GLuint billboard_uniformID_VP;
static GLuint billboard_uniformID_sampler;

arraylist entity_register;


int init_entities()
{
	/* render_init() must have run before this */
	/* Set the billboard shaders up */
	programID_billboard = LoadShaders(
			"shaders/Billboard_Vertexshader.vertexshader",
			"shaders/Billboard_Fragmentshader.fragmentshader");
	glGenBuffers(1, &vertexbuffer_billboard);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_billboard);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square2Din3D_data), square2Din3D_data, GL_STATIC_DRAW);

	/* get the unifom locations */
	billboard_uniformID_scale = glGetUniformLocation(
			programID_billboard, "scale");
	billboard_uniformID_position = glGetUniformLocation(
			programID_billboard, "position");
	billboard_uniformID_CameraRight_worldspace = glGetUniformLocation(
			programID_billboard, "CameraRight_worldspace");
	billboard_uniformID_VP = glGetUniformLocation(
			programID_billboard, "VP");
	billboard_uniformID_sampler = glGetUniformLocation(
			programID_billboard, "my_sampler");

	arraylist_init(	&entity_register, 
			sizeof(struct entity_index_card), 
			1 );
	return 0;
}

void render_entities(float view[4][4], float projection[4][4])
{
	float vp[4][4];
	mult_mat4_mat4(projection, view, vp);

	//glDisable(GL_DEPTH_TEST);

	//SDL_Log("RENDER ENTITIES!");

	/* loop over all entities */
	for(int i = 0; i < entity_register.used_units; i++) {
		/* once per entity type */
		struct entity_index_card* eic =
			arraylist_get(&entity_register, i);
		//SDL_Log("RENDER %s!", eic->name);
		switch(eic->rt) {
			case RENDER_SPRITE:
				//SDL_Log("Spreit");
				glUseProgram(programID_billboard);
				glActiveTexture(GL_TEXTURE0);
				glUniform1i(billboard_uniformID_sampler, 0);
				//glDisable(GL_BLEND);
				glUniformMatrix4fv(
						billboard_uniformID_VP,
						1,
						GL_FALSE,
						vp);
				glUniform1f(
						billboard_uniformID_scale,
						eic->em.sp.size);
				glUniform3f(
						billboard_uniformID_CameraRight_worldspace,
						view[0][0],
						view[1][0],
						view[2][0]);
				glBindTexture(
						GL_TEXTURE_2D,
						eic->em.sp.textureID);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				//SDL_Log("textureID:%i", eic->em.sp.textureID);
				break;
			case RENDER_BLOCK:
				break;
			case RENDER_MODEL:
				break;
			default:
				sdldie("Invalid render type of entity.");
		}

		for(int j = 0; j < eic->entity_pointers.used_units; j++) {
			/* once per entity */
			struct live_entity** lepp = arraylist_get(&eic->entity_pointers, j);
			struct live_entity* lep = *lepp;

			//SDL_Log("RENDER ENTITY %s #%i!", eic->name, j);

			/* set further values for that shader */
			switch(eic->rt) {
				case RENDER_SPRITE:
					glUniform3f(
							billboard_uniformID_position,
							lep->pos[0],
							lep->pos[1],
							lep->pos[2]);
					glEnableVertexAttribArray(0);
					glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_billboard);
					glVertexAttribPointer(
							0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
							3,                  // size
							GL_FLOAT,           // type
							GL_FALSE,           // normalized?
							0,                  // stride
							(void*)0            // array buffer offset
							);
					//glDisable(GL_CULL_FACE);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					//glEnable(GL_CULL_FACE);
					glDisableVertexAttribArray(0);
					break;
				case RENDER_BLOCK:
					break;
				case RENDER_MODEL:
					break;
				default:
					sdldie("Invalid render type of entity.");
			}
		}

	}
}

/* Copies the entity index card into the entity index
 * making it available for spawning into the game world.
 *
 * This function initializes the "entities" arraylist
 * element of the index card itself so old arraylists,
 * passed to this function will not overlapp with the
 * storage of the games live entities.
 *
 * Returns a pointer to the index card in the index. */
struct entity_index_card *register_entity(struct entity_index_card *entry)
{
	struct entity_index_card *ic;
	arraylist_append(&entity_register, entry);

	ic = arraylist_get(&entity_register,
			entity_register.used_units - 1);

	arraylist_init(	&ic->entity_pointers,
			sizeof(struct live_entity *),
			8 );
	return ic;
}

struct live_entity *spawn_entity(struct entity_index_card *type,
		long chunk[2], double pos[3])
{
	struct live_entity *lep = malloc(sizeof(struct live_entity));
	SDL_Log("SPAWN!");
	lep->type = type;
	memcpy(lep->chunk_offset, chunk, 2 * sizeof(long));
	memcpy(lep->pos, pos, 3 * sizeof(double));


	arraylist_append(&type->entity_pointers, &lep);

	return lep;
}

/* Returns 0 on success, -1 on failure (entity to delete
 * does not exist). */
int destroy_entity(struct live_entity *entity)
{
	struct entity_index_card *index_card = entity->type;
	arraylist *entity_pointers = &index_card->entity_pointers;
	/* find entity in entity pointers and delete it
	 * otherwise return -1 */
	for(int i = 0; i < entity_pointers->used_units; i++) {
		struct live_entity **lepp =
			arraylist_get(entity_pointers, i);
		struct live_entity *lep = *lepp;
		if(lep == entity) {
			/* TODO entity death routines */
			/* entity exisrts, and will be deleted */
			arraylist_del_element(entity_pointers, i);
			/* also free the entities memory */
			free(entity);
			return 0;
		}
	}
	/* entity was not found */
	return -1;
}
