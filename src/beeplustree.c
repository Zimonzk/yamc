#include "beeplustree.h"

#include <stdio.h>
#include <string.h>
#include <endian.h>

#include "plogger.h"
#include "zimonzk/lists.h"
#include "toolbox.h"

int beept_init(struct beept *bpt, char *path)
{
	if((bpt->f = fopen64(path, "r+b")) == 0) {
		return -1;
	}

	/* if the file is empty, place the root node */

	return 0;
}

void beept_close(struct beept *bpt)
{
	fclose(bpt->f);
	memset(bpt, 0, sizeof(struct beept));
}

static void bpt_split_node(struct beept *bpt, struct bpt_node *cno,
		uint64_t rval[2], off64_t chld,
		arraylist *path, int i, uint8_t is_leaf);
static void node_insert(struct beept *bpt, uint64_t rval[2], off64_t chld, int i,
		arraylist *path);
static void bpt_place_and_shift_down(struct bpt_node *cno, 
		uint64_t rval[2], off64_t chld, int i);
static void bpt_node_to_disk(struct beept *bpt, struct bpt_node *cno,
		off64_t addr);
static void bpt_node_from_disk_here(struct beept *bpt, struct bpt_node *cno);




static void bpt_split_node(struct beept *bpt, struct bpt_node *cno,
		uint64_t rval[2], off64_t chld,
		arraylist *path, int i, uint8_t is_leaf)
{
	/* allocate new node */
	off64_t nodeo;
	struct bpt_node newnode = {}, parent = {};
	uint64_t newrval[2];
	newnode.is_leaf = is_leaf;

	fseeko64(bpt->f, SEEK_END, 0);
	nodeo = ftello64(bpt->f);

	if(i > 16) {
		/* key will be placed in the new node */
		newnode.nrval = 15;
	} else {
		/* key will be placed in the old node */
		newnode.nrval = 16;
	}

	/*TODO something about extra router value that goes to the parent*/
	/* move second half of keys and values */
	memcpy(newnode.rvals, cno->rvals+(2*(31-newnode.nrval)), 2*8*newnode.nrval);
	memcpy(newnode.children, cno->children+31-newnode.nrval, 8*newnode.nrval);
	cno->nrval = 31 - newnode.nrval;

	/* place key and value and shift down */
	if(i > 16) {
		bpt_place_and_shift_down(&newnode, rval, chld, i-(31-newnode.nrval));
	} else {
		bpt_place_and_shift_down(cno, rval, chld, i);
	}


	/* if we are in a non-leaf the last router value becomes useless in the
	 * current node, since the last child must not have a router value.
	 * becuse of this we have to move that router value up to the parent. 
	 * this also allows us to properly link to our old, now halved, node
	 * which of course should be linked to by its old highest router value
	 * which again is the one we have to move anyways. */
	memcpy(newrval, cno->rvals[cno->nrval-1], 2*8);
	if(newnode.is_leaf == 0)
	{
		cno->nrval--;
	}
	/* otherwise we only copy the "rval" that is actually a key
	 * into the parent. */


	/* write new node to disk */
	bpt_node_to_disk(bpt, &newnode, nodeo);

	/* update the parent */
	if(path->used_units == 0) {
		/* we are in the root node */
		/* in this case we have to split it into two entirely new nodes
		 * and only link to those from the root. */
		
		/* at this point we already have split off one new node and put
		 * the values in. so we only create one more node and copy the
		 * content of the current root into it.
		 * this basically means just writing the current root out to the
		 * end of the file and remembering its new address.
		 * after that we change the current root to only contain the
		 * links to the two new nodes. */
		off64_t nodeo2;
		struct bpt_node newroot;

		/* write to disk */
		fseeko64(bpt->f, SEEK_END, 0);
		nodeo2 = ftello64(bpt->f);
		bpt_node_to_disk(bpt, cno, nodeo2);

		/* now make a new node to overwrite the root on file with */
		newroot.is_leaf = 0;
		newroot.nrval = 1;
		memcpy(newroot.rvals[0], newrval, 2*8);
		newroot.children[0] = nodeo2;
		newroot.children[1] = nodeo;

		/* overwrite root node */
		bpt_node_to_disk(bpt, &newroot, 0);

	} else {
		/* write original changed node back to disk */
		bpt_node_to_disk(bpt, cno, *(off64_t *)arraylist_get(path, 
				path->used_units-1));

		/* go back in path */
		arraylist_del_element(path, path->used_units-1);
		/* read the parent node */
		fseeko64(bpt->f, SEEK_SET,
				*(off64_t*)arraylist_get(path,
					path->used_units-1));
		bpt_node_from_disk_here(bpt, &parent);

		/* node has space */
		/* insert router value */
		for(int i = 0; i < parent.nrval; i++) {
			if(xz_greater(parent.rvals[i], newrval)) {
				if(parent.nrval == 31) {
					/* node full */
					bpt_split_node(bpt, &parent, newrval,
							nodeo, path, i, 0);
				} else {
					/* node has space */
					bpt_place_and_shift_down(&parent,
							newrval,
							nodeo,
							i);
				}
			}
		}
	}

}

static void node_insert(struct beept *bpt, uint64_t rval[2], off64_t chld, int i,
		arraylist *path)
{
	struct bpt_node *cno = &bpt->curr_node;

	if(cno->nrval < 31) {
		/* there is space left in node */

		bpt_place_and_shift_down(cno, rval, chld, i);

		/* write the node back to the disk */
		bpt_node_to_disk(bpt, cno, *(off64_t *)arraylist_get(path, 
					path->used_units-1));
	} else {
		/* no space left in node */
		/* split node */
		bpt_split_node(bpt, cno, rval, chld, path, i, 1);
	}
}

static void bpt_place_and_shift_down(struct bpt_node *cno, 
		uint64_t rval[2], off64_t chld, int i)
{
	uint64_t tempr[2];
	off64_t tempc;

	for(int j = i; j < cno->nrval; j++) {
		memcpy(tempr, cno->rvals[j], 2*8);
		tempc = cno->children[j];

		memcpy(cno->rvals[j], rval, 2*8);
		cno->children[j] = chld;

		memcpy(rval, tempr, 2*8);
		chld = tempc;
	}
	tempc = cno->children[cno->nrval];

	memcpy(cno->rvals[cno->nrval], rval, 2*8);
	cno->children[cno->nrval] = chld;

	cno->nrval++;

	cno->children[cno->nrval] = tempc;
}

static void bpt_node_to_disk(struct beept *bpt, struct bpt_node *cno,
		off64_t addr)
{
	uint8_t data[755];
	fseeko64(bpt->f, SEEK_SET, addr);

	memcpy(data, &cno->is_leaf, 1);

	cno->is_leaf = htole16(cno->is_leaf);
	memcpy(data+1, &cno->nrval, 2);

	for(int i = 0; i < 31; i++) {
		cno->rvals[i][0] = htole64(cno->rvals[i][0]);
		cno->rvals[i][1] = htole64(cno->rvals[i][1]);
	}
	memcpy(data+3, &cno->rvals, 2*31*8);

	for(int i = 0; i < 32; i++) {
		cno->children[i] = htole64(cno->children[i]);
	}
	memcpy(data+2*31*8, &cno->children, 32*8);

	if(fwrite(data, sizeof(data), 1, bpt->f) != 1) {
		yamc_terminate(-124, "Could not write to beeplustree.");
	}
}

static void bpt_node_from_disk_here(struct beept *bpt, struct bpt_node *cno)
{
	uint8_t data[755];
	/* yet to be implemented */
	if(fread(data, sizeof(data), 1, bpt->f) != 1) {
		yamc_terminate(-1,
				"Could not read a node from a beeplustree.");
	}

	memcpy(&cno->is_leaf, data, 1);

	memcpy(&cno->nrval, data+1, 2);
	cno->nrval = le16toh(cno->nrval);

	memcpy(&cno->rvals, data+3, 2*31*8);
	for(int i = 0; i < 31; i++) {
		cno->rvals[i][0] = le64toh(cno->rvals[i][0]);
		cno->rvals[i][1] = le64toh(cno->rvals[i][1]);
	}

	memcpy(&cno->children, data+2*31*8, 32*8);
	for(int i = 0; i < 32; i++) {
		cno->children[i] = le64toh(cno->children[i]);
	}
}


uint64_t bpt_get(struct beept *bpt, uint64_t key[2])
{
	struct bpt_node *cno = &bpt->curr_node;

	fseeko64(bpt->f, SEEK_SET, 0);

	while(1) {
		bpt_node_from_disk_here(bpt, cno);

		if(cno->is_leaf) {
			/* search for match */
			for(int i = 0; i < cno->nrval; i++) {
				if(xz_equal(cno->rvals[i], key)) {
					/* In a leaf child nr. X is the value
					 * to key nr. X.
					 * That mens that the last possible
					 * child in a leaf, although it is
					 * stored on disk has no meaning. */
					return cno->children[i];
				}
			}

			return 0; /*key nonexistent*/
		} else {
			/* search for router value that is bigger than or equal
			 * to key */
			off64_t n = cno->nrval;
			for(int i = 0; i < cno->nrval; i++) {
				if(xz_greater(cno->rvals[i], key) ||
						xz_equal(cno->rvals[i], key)) {
					n = i;
				}
			}
			fseeko64(bpt->f, SEEK_SET, cno->children[n]);
		}
	}
}

int bpt_add(struct beept *bpt, uint64_t key[2], uint64_t value)
{
	struct bpt_node *cno = &bpt->curr_node;
	off64_t cur = 0;
	arraylist path;

	arraylist_init(&path, sizeof(off64_t), 8);

	fseeko64(bpt->f, SEEK_SET, 0);
	/* don't put the root node into path */
	/* arraylist_append(&path, &cur); */

	while(1) {
		bpt_node_from_disk_here(bpt, cno);

		if(cno->is_leaf) {
			uint64_t match = 0;
			/* search for match */
			for(int i = 0; i < cno->nrval; i++) {
				if(xz_equal(cno->rvals[i], key)) {
					/* In a leaf child nr. X is the value
					 * to key nr. X.
					 * That mens that the last possible
					 * child in a leaf, although it is
					 * stored on disk has no meaning. */
					match = cno->children[i];
					arraylist_delete(&path);
					return -1; /*key exists, can't add.*/
				}
				if(xz_greater(key, cno->rvals[i])) {
					/* found the place where the new key
					 * belongs */
					node_insert(bpt, key, value, i, &path);

					arraylist_delete(&path);
					return 0;
				}
			}
			/* all keys are bigger than the new key
			 * THIS SHOULD NEVER HAPPEN */
			yamc_terminate(-123, "Reached forbidden part in code."
					" Possibly broken beeplustree file.");

		} else {
			/* search for router value, that is bigger than or equal
			 * to key */
			off64_t n = cno->nrval;
			for(int i = 0; i < cno->nrval; i++) {
				if(xz_greater(cno->rvals[i], key) ||
						xz_equal(cno->rvals[i], key)) {
					n = i;
				}
			}
			fseeko64(bpt->f, SEEK_SET, cno->children[n]);
			arraylist_append(&path, &cno->children[n]);
		}
	}

	arraylist_delete(&path);
	return -2;
}

