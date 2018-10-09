#include "block.h"

#include <gl/gl.h>

#include "zimonzk/lists.h"

static arraylist registered_blocks = {};

unsigned int register_block(const char* name, const char* displayname, const unsigned int hardness, const unsigned int* texis)
{
	static int list_inited = 0;
	struct block_register br = {};
	if(!list_inited) {
		arraylist_init(&registered_blocks, sizeof(struct block_register), 1);
		list_inited = 1;
	}
	br.name = strdup(name);
	br.displayname = strdup(displayname);
	br.hardness = hardness;
	memcpy(br.texis, texis, sizeof(br.texis));
	
	arraylist_append(&registered_blocks, &br);
	return (unsigned int) registered_blocks.used_units; /* first registered block will have id 1 but its location un the arraylist will be 0.
				    this is in order to have id 0 for "air" which is always present.
				    so position in list = id - 1 
				    also, we will get problems if we register more blocks tha an uint can handle (including the air) */
}

struct block_register* get_registered_block(unsigned int block_id)
{
	return (struct block_register*) arraylist_get(&registered_blocks, block_id - 1);
}

int block_set_interaction(interaction_cb icb)
{

	return 0;
}
int block_set_placement(placement_cb pcb)
{

	return 0;
}
