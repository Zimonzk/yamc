#include "beeplustree.h"

#include <stdio.h>
#include <string.h>
#include <endian.h>

#include "plogger.h"
#include "zimonzk/lists.h"
#include "toolbox.h"


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




int beept_init(struct beept *bpt, char *path)
{
	if((bpt->f = fopen64(path, "r+b")) == 0) {
		tlog(5, "PEPP");
		if((bpt->f = fopen64(path, "w+b")) == 0) {
			return -1;
		}
	}

	fseeko64(bpt->f, 0, SEEK_END);

	if(ftello64(bpt->f) == 0) {
		/* if the file is empty, place the root node */
		/* also link to one empty child node 
		 * (this should work in our implementation) */
		struct bpt_node root = {}, chld = {};
		root.is_leaf = 0;
		root.nrval = 0;
		root.children[0] = 755;

		chld.is_leaf = 1;
		chld.nrval = 0;

		tlog(5, "meeppp");

		bpt_node_to_disk(bpt, &root, 0);
		bpt_node_to_disk(bpt, &chld, 755);
	}
	return 0;
}

void beept_close(struct beept *bpt)
{
	fclose(bpt->f);
	memset(bpt, 0, sizeof(struct beept));
}

static void bpt_split_node(struct beept *bpt, struct bpt_node *cno,
		uint64_t rval[2], off64_t chld,
		arraylist *path, int i, uint8_t is_leaf)
{
	/* allocate new node */
	off64_t nodeo;
	struct bpt_node newnode = {}, parent = {};
	uint64_t newrval[2];
	newnode.is_leaf = is_leaf;

	fseeko64(bpt->f, 0, SEEK_END);
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
		struct bpt_node newroot = {};

		/* write to disk */
		fseeko64(bpt->f, 0, SEEK_END);
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
		off64_t poff;
		/* write original changed node back to disk */
		bpt_node_to_disk(bpt, cno, *(off64_t *)arraylist_get(path, 
					path->used_units-1));

		/* go back in path */
		arraylist_del_element(path, path->used_units-1);

		if(path->used_units == 0) {
		/* our parent is the root node (which is not in path so
		 * we use its known address: 0)*/
			poff = 0;
		} else {
			/* read the parent node */
			poff = *(off64_t*) arraylist_get(path,
					path->used_units-1);
		}
		fseeko64(bpt->f, poff, SEEK_SET);
		bpt_node_from_disk_here(bpt, &parent);

		/* insert router value */
		for(int i = 0; i < parent.nrval; i++) {
			if(xz_greater(parent.rvals[i], newrval)) {
				if(parent.nrval == 31) {
					/* node full */
					bpt_split_node(bpt, &parent, newrval,
							nodeo, path, i, 0);
					/* split already saves all nodes */
				} else {
					/* node has space */
					bpt_place_and_shift_down(&parent,
							newrval,
							nodeo,
							i);
					/* in this case, the node also needs
					 * to be written back here */
					bpt_node_to_disk(bpt, &parent, poff);
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

		tlog(6, "has space");

		bpt_place_and_shift_down(cno, rval, chld, i);

		/* write the node back to the disk */
		bpt_node_to_disk(bpt, cno, *(off64_t *)arraylist_get(path, 
					path->used_units-1));
	} else {
		/* no space left in node */
		tlog(6, "has no space");
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
		/* backup current */
		memcpy(tempr, cno->rvals[j], 2*8);
		tempc = cno->children[j];

		/* add new */
		memcpy(cno->rvals[j], rval, 2*8);
		cno->children[j] = chld;

		/* make backup into "new" */
		memcpy(rval, tempr, 2*8);
		chld = tempc;
	}
	/* backup extra child for nonleafs */
	tempc = cno->children[cno->nrval];

	/* put "new" into new last place */
	memcpy(cno->rvals[cno->nrval], rval, 2*8);
	cno->children[cno->nrval] = chld;

	/* update used values count */
	cno->nrval++;

	/* also put the extra child into the new place */
	cno->children[cno->nrval] = tempc;
}

static void bpt_node_to_disk(struct beept *bpt, struct bpt_node *cno,
		off64_t addr)
{
	uint8_t data[755];
	fseeko64(bpt->f, addr, SEEK_SET);

	data[0] = cno->is_leaf;
	tlog(6, "is leaf? %i", cno->is_leaf);

	cno->nrval = htole16(cno->nrval);
	memcpy(data+1, &cno->nrval, 2);

	for(int i = 0; i < 31; i++) {
		cno->rvals[i][0] = htole64(cno->rvals[i][0]);
		cno->rvals[i][1] = htole64(cno->rvals[i][1]);
	}
	memcpy(data+3, &cno->rvals, 2*31*8);

	for(int i = 0; i < 32; i++) {
		cno->children[i] = htole64(cno->children[i]);
	}
	memcpy(data+3+2*31*8, &cno->children, 32*8);

	tlog(6, "write at %lli", addr);
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

	cno->is_leaf = data[0];

	memcpy(&cno->nrval, data+1, 2);

	cno->nrval = le16toh(cno->nrval);

	memcpy(&cno->rvals, data+3, 2*31*8);
	for(int i = 0; i < 31; i++) {
		cno->rvals[i][0] = le64toh(cno->rvals[i][0]);
		cno->rvals[i][1] = le64toh(cno->rvals[i][1]);
	}

	memcpy(&cno->children, data+3+2*31*8, 32*8);
	for(int i = 0; i < 32; i++) {
		cno->children[i] = le64toh(cno->children[i]);
	}
}


uint64_t bpt_get(struct beept *bpt, uint64_t key[2])
{
	struct bpt_node *cno = &bpt->curr_node;

	fseeko64(bpt->f, 0, SEEK_SET);

	while(1) {
		bpt_node_from_disk_here(bpt, cno);

		if(cno->is_leaf) {
			/* search for match */
			for(int i = 0; i < cno->nrval; i++) {
				tlog(6, "i %i, nrval %i, rval %lli %lli, key %lli %lli",
						i, cno->nrval,
						cno->rvals[i][0], cno->rvals[i][1],
						key[0], key[1]);
				if(xz_equal(cno->rvals[i], key)) {
					/* In a leaf child nr. X is the value
					 * to key nr. X.
					 * That mens that the last possible
					 * child in a leaf, although it is
					 * stored on disk has no meaning. */
					tlog(6, "key found");
					return cno->children[i];
				}
			}

			tlog(6, "key not found");

			return 0; /*key nonexistent*/
		} else {
			/* search for router value that is bigger than or equal
			 * to key */
			off64_t n = cno->nrval;
			for(int i = 0; i < cno->nrval; i++) {
				tlog(6, "rval #%i is %lli %lli", i, cno->rvals[0], cno->rvals[1]);
				if(xz_greater(cno->rvals[i], key) ||
						xz_equal(cno->rvals[i], key)) {
					tlog(6, "rval matches key %lli %lli", key[0], key[1]);
					n = i;
					break;
				}
			}
			tlog(6, "going to node at %lli", cno->children[n]);
			fseeko64(bpt->f, cno->children[n], SEEK_SET);
		}
	}
}

int bpt_add(struct beept *bpt, uint64_t key[2], uint64_t value)
{
	struct bpt_node *cno = &bpt->curr_node;
	off64_t cur = 0;
	arraylist path;

	arraylist_init(&path, sizeof(off64_t), 8);

	fseeko64(bpt->f, 0, SEEK_SET);
	/* don't put the root node into path */
	/* arraylist_append(&path, &cur); */

	while(1) {
		bpt_node_from_disk_here(bpt, cno);

		if(cno->is_leaf) {
			/* search for match */
			for(int i = 0; i < cno->nrval; i++) {
				if(xz_equal(cno->rvals[i], key)) {
					/*key exists, can't add.*/
					arraylist_delete(&path);
					return -1;
				}
				if(xz_greater(cno->rvals[i], key)) {
					/* found the place where the new key
					 * belongs */
					tlog(6, "adding nonlast");
					node_insert(bpt, key, value, i, &path);

					arraylist_delete(&path);
					return 0;
				}
			}
			/* all keys are smaller than the new key
			 * add new key at the end
			 * the only time we should be getting here is when
			 * adding the new highest key of the tree */
			tlog(6, "adding last");

			node_insert(bpt, key, value, cno->nrval, &path);

			arraylist_delete(&path);
			return 0;
		} else {
			/* search for router value, that is bigger than or equal
			 * to key */
			off64_t n = cno->nrval;
			for(int i = 0; i < cno->nrval; i++) {
				if(xz_greater(cno->rvals[i], key) ||
						xz_equal(cno->rvals[i], key)) {
					n = i;
					break;
				}
			}

			fseeko64(bpt->f, cno->children[n], SEEK_SET);
			arraylist_append(&path, &cno->children[n]);
		}
	}

	arraylist_delete(&path);
	return -2;
}

