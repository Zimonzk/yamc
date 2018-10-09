#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#define BLOCKID_MAX 4096

#define BLOCK_OPAQUE 0b1

typedef int (*interaction_cb)(long x, long z, unsigned int id);
typedef int (*placement_cb)(long x, long z, unsigned int id);

typedef struct s_block
{
    unsigned int id;
    unsigned char meta;
    unsigned char properties;
} block;

struct block_register
{
	char* name;
	char* displayname;
	unsigned int hardness;
	unsigned int texis[6];
	interaction_cb icb;
	placement_cb pcb;
};

enum block_side {FRONT, BACK, LEFT, RIGHT, UPPER, LOWER};

/* make a block known to the game
 * registered blocks can be used in the wold and as items
 * returns the id this block got assigned*/
unsigned int register_block(const char* name, const char* displayname, const unsigned int hardness, const unsigned int* texis);
/* get a block_register for your block_id */
struct block_register* get_registered_block(unsigned int block_id);
int block_set_interaction(interaction_cb icb);
int block_set_placement(placement_cb pcb);

#endif // BLOCK_H_INCLUDED
