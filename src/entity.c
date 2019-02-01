#include "entity.h"

#include <string.h>

#include "zimonzk/lists.h"


static const GLfloat square2Din3D_data[] = {
	-0.5, -0.5, 0.0,
	0.5, -0.5, 0.0, 
	0.5, 0.5, 0.0,

	-0.5, -0.5, 0.0,
	0.5, 0.5, 0.0,
	-0.5, 0.5, 0.0
};

arraylist entity_register;


int init_entities()
{
	/* Set the billboard shaders up */

	arraylist_init(	&entity_register, 
			sizeof(struct entity_index_card), 
			1 );
	return 0;
}

void render_entities()
{
	//select correct shader
	
	//set values for that shader
	

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
		struct live_entity *lep =
		       arraylist_get(entity_pointers, i);
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
