#ifndef __ENTITY_H_INCLUDED__
#define __ENTITY_H_INCLUDED__

#include "zimonzk/lists.h"

#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/gl.h>

enum render_type {RENDER_SPRITE, RENDER_BLOCK, RENDER_MODEL};

struct sprite_model
{
	double size;
	GLuint textureID;
};

struct model_3d
{

};

union entity_model
{
	struct sprite_model sp;
	struct model_3d m3d;
};

enum colision_geometry_type {COLISION_SPHERE, COLISION_CUBE, CLOISION_CUBOID};

union colision_geometry
{
	double radius;
	double side;
	double sides[3];
};

struct colision_primitive
{
	enum colision_geometry_type gt;
	union colision_geometry geometry;
};

struct colision_model
{
	unsigned int num_primitives;
	struct colision_primitive* primitives;
};

struct entity_index_card
{
	/* TODO make metadata possible */
	char* name; /* must be unique */
	char* displayname;
	int health; /* -1 for entities that don't have health */
	enum render_type rt;	
	union entity_model em;
	struct colision_model;
	arraylist entity_pointers;	
};

struct live_entity
{
	struct entity_index_card *type;
	long chunk_offset[2];
	double pos[3];
	int health;
};

int init_entities(void);

/* to be used only in the rendering loop once per frame */
void render_entities(float view[4][4], float projection[4][4]);

struct entity_index_card *register_entity(struct entity_index_card *entry);

struct live_entity *spawn_entity(struct entity_index_card *type,
		long chunk[2], double pos[3]);

int destroy_entity(struct live_entity *entity);

#endif
