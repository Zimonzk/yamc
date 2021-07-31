#include "beeplustree.h"

#include <stdio.h>
#include <string.h>
#include <endian.h>

#include "plogger.h"
#include "zimonzk/lists.h"
#include "toolbox.h"

#include <SDL2/SDL.h>


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
		tlog(6, "PEPP");
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

		tlog(6, "meeppp");

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
	off64_t nodeo;
	struct bpt_node newnode = {}, parent = {};
	uint64_t newrval[2];
	newnode.is_leaf = is_leaf;

	/* allocate new node */
	fseeko64(bpt->f, 0, SEEK_END);
	nodeo = ftello64(bpt->f);

	if(i > 15) {
		/* key will be placed in the new node */
		newnode.nrval = 15;
	} else {
		/* key will be placed in the old node */
		newnode.nrval = 16;
	}

	/* move second half of keys and values */
	memcpy(newnode.rvals, cno->rvals[31-newnode.nrval], 2*8*newnode.nrval);
	tlog(6, "first rval in the new node will be %llu %llu", newnode.rvals[0][0], newnode.rvals[0][1]);
	memcpy(newnode.children, &cno->children[31-newnode.nrval], 8*(newnode.nrval+1));
	cno->nrval = 31 - newnode.nrval;

	tlog(6, "placing the new rval+chid into node.");
	/* place key and value and shift down */
	if(i > 15) {
		/* place in new node */
		tlog(6, "placing new rval %llu %llu and child %llu in new node at i %i",
				rval[0], rval[1], chld, i-(31-newnode.nrval));	
		bpt_place_and_shift_down(&newnode, rval, chld, i-(31-newnode.nrval));
	} else if((i == 15) && (is_leaf == 0)) {
		/* the new rval and new node go in different nodes */
		memcpy(cno->rvals[15], rval, 2*8);
		cno->nrval++;

		newnode.children[0] = chld;
	} else {
		/* place in old node */
		tlog(6, "placing in the old node at i %i", i);
		bpt_place_and_shift_down(cno, rval, chld, i);
	}
	tlog(6, "the old node now contains the router values: ");
	for(int dbg = 0; dbg < cno->nrval; dbg++) {
		tlog(6, "%llu %llu", cno->rvals[dbg][0], cno->rvals[dbg][1]);
	}
	tlog(6, "the new node now contains the router values: ");
	for(int dbg = 0; dbg < newnode.nrval; dbg++) {
		tlog(6, "%llu %llu", newnode.rvals[dbg][0], newnode.rvals[dbg][1]);
	}

	/* if we are in a non-leaf the last router value becomes useless in the
	 * current node, since the last child must not have a router value.
	 * becuse of this we have to move that router value up to the parent. 
	 * this also allows us to properly link to our old, now halved, node
	 * which of course should be linked to by its old highest router value
	 * which again is the one we have to move anyways. */
	memcpy(newrval, cno->rvals[cno->nrval-1], 2*8);
	tlog(6, "newrval is %llu %llu", newrval[0], newrval[1]);
	if(newnode.is_leaf == 0)
	{
		cno->nrval--;
	}
	/* otherwise we only copy the "rval" that is actually a key
	 * into the parent. */

	/* write new node to disk */
	bpt_node_to_disk(bpt, &newnode, nodeo);

	tlog(6, "path used units %lu", path->used_units);

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

		tlog(5, "splitting root");

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
			tlog(6, "parent is root");
			poff = 0;
		} else {
			/* read the parent node */
			poff = *(off64_t*) arraylist_get(path,
					path->used_units-1);
		}
		fseeko64(bpt->f, poff, SEEK_SET);
		bpt_node_from_disk_here(bpt, &parent);

		tlog(6, "the parent has %i router values", parent.nrval);

		/* insert router value */
		for(int i = 0; i < parent.nrval; i++) {
			if(xz_greater(parent.rvals[i], newrval)) {
				if(parent.nrval == 31) {
					tlog(6, "the parent node is full");
					/* node full */
					bpt_split_node(bpt, &parent, newrval,
							nodeo, path, i, 0);
					/* split already saves all nodes */
					return;
				} else {
					/* node has space */
					bpt_place_and_shift_down(&parent,
							newrval,
							nodeo,
							i);
					/* in this case, the node also needs
					 * to be written back */
					bpt_node_to_disk(bpt, &parent, poff);
					return;
				}
			}
		}
		/* TODO clean this and simmilar loop constructs up
		 * so that the body ist not repeated below the loop itself.
		 * somehow put that case also into the loop. */
		tlog(6, "newrval now is %llu %llu", newrval[0], newrval[1]);
		/* no router values yet, or all values were smaller */
		if(parent.nrval == 31) {
			/* node full */
			tlog(6, "the parent node is full");

			bpt_split_node(bpt, &parent, 
					newrval, nodeo, path, parent.nrval, 0);

		} else {
			bpt_place_and_shift_down(&parent, newrval, nodeo, parent.nrval);
			/* in this case, the node also needs
			 * to be written back */
			bpt_node_to_disk(bpt, &parent, poff);
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
	uint64_t rval_local[2];
	uint64_t tempr[2];
	off64_t tempc;

	memcpy(rval_local, rval, 2*8);

	for(int j = i; j < cno->nrval; j++) {
		/* backup current */
		memcpy(tempr, cno->rvals[j], 2*8);
		/* in leaves, the value is at the index f the key.
		 * otherwise the router value goes in the next child slot */
		tempc = cno->children[j+(cno->is_leaf ? 0 : 1)];

		/* add new */
		memcpy(cno->rvals[j], rval_local, 2*8);
		cno->children[j+(cno->is_leaf ? 0 : 1)] = chld;

		/* make backup into "new" */
		memcpy(rval_local, tempr, 2*8);
		chld = tempc;
	}
	/* put "new" into new last place */
	memcpy(cno->rvals[cno->nrval], rval_local, 2*8);
	tlog(6, "wrote last rval of %llu %llu", cno->rvals[cno->nrval][0], cno->rvals[cno->nrval][1]);
	cno->children[cno->nrval+(cno->is_leaf ? 0 : 1)] = chld;

	/* update used values count */
	cno->nrval++;
}

static void bpt_node_to_disk(struct beept *bpt, struct bpt_node *cno,
		off64_t addr)
{
	uint8_t data[755];
	fseeko64(bpt->f, addr, SEEK_SET);

	data[0] = cno->is_leaf;
	//tlog(6, "is leaf? %i", cno->is_leaf);

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

	//tlog(6, "write at %llx", addr);
	if(fwrite(data, sizeof(data), 1, bpt->f) != 1) {
		yamc_terminate(-124, "Could not write to beeplustree.");
	}
}

static void bpt_node_from_disk_here(struct beept *bpt, struct bpt_node *cno)
{
	uint8_t data[755];
	/* yet to be implemented */
	if(fread(data, sizeof(data), 1, bpt->f) != 1) {
		yamc_terminate(-1, "Could not read a node from a beeplustree.");
	}

	cno->is_leaf = data[0];

	memcpy(&cno->nrval, data+1, 2);

	cno->nrval = le16toh(cno->nrval);

	memcpy(&cno->rvals, data+3, 2*31*8);
	for(int i = 0; i < 31; i++) {
		cno->rvals[i][0] = le64toh(cno->rvals[i][0]);
		cno->rvals[i][1] = le64toh(cno->rvals[i][1]);
	}
	//tlog(6, "first rval read %llu %llu", cno->rvals[0][0], cno->rvals[0][1]);

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
			tlog(6, "leaf searching key for %llu %llu", key[0], key[1]);
			for(int i = 0; i < cno->nrval; i++) {
				tlog(6, "rval %llu %llu",
						cno->rvals[i][0],
						cno->rvals[i][1]);
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

			tlog(5, "key not found");

			return 0; /*key nonexistent*/
		} else {
			/* search for router value that is bigger than or equal
			 * to key */
			tlog(6, "nonleaf searching router value for key %llu %llu",
					key[0], key[1]);
			off64_t n = cno->nrval;
			for(int i = 0; i < cno->nrval; i++) {
				tlog(6, "rval %llu %llu", cno->rvals[i][0], cno->rvals[i][1]);
				if(xz_greater(cno->rvals[i], key) ||
						xz_equal(cno->rvals[i], key)) {
					tlog(6, "rval matches key");
					n = i;
					break;
				}
			}
			tlog(6, "going to node at %llx", cno->children[n]);
			if(cno->children[n] == 0) {
				yamc_terminate(-132, "nullpointer as child.");
			}
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
				tlog(6, "leaf rval %llu, %llu", cno->rvals[i][0], cno->rvals[i][1]);
				if(xz_equal(cno->rvals[i], key)) {
					/*key exists, can't add.*/
					arraylist_delete(&path);
					tlog(6, "double add k %llu %llu, v %llu %llu",
							key[0], key[1], cno->rvals[i][0], cno->rvals[i][1]);
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
			tlog(6, "nonleaf, searching correct router value");
			for(int i = 0; i < cno->nrval; i++) {
				tlog(6, "rval %llu %llu low-child %llu high-child %llu",
						cno->rvals[i][0], cno->rvals[i][1],
						cno->children[i], cno->children[i+1]);
				if(xz_greater(cno->rvals[i], key) ||
						xz_equal(cno->rvals[i], key)) {
					tlog(6, "it matched");
					n = i;
					break;
				}
			}

			tlog(6, "going to node at %llx", cno->children[n]);
			/* all router values smaller. last child is used. */
			if(cno->children[n] == 0) {
				yamc_terminate(-132, "nullpointer as child.");
			}
			fseeko64(bpt->f, cno->children[n], SEEK_SET);
			arraylist_append(&path, &cno->children[n]);
		}
	}

	arraylist_delete(&path);
	return -2;
}

/*void beeplustest(void)
{
#define ITERR 32LL
#define BEETEST_RANDOM
	uint32_t testticks = SDL_GetTicks();
	struct beept testbee = {};
	int t = beept_init(&testbee, "test.beept");
	uint64_t testbeek[2] = {(uint64_t) 11L, (uint64_t) 777L};
	uint64_t v = 0;
	int count = 1;
	uint64_t value;
	
	tlog(5, "bpt inited. took %li ms.", SDL_GetTicks() - testticks);
	tlog(5, "Beept init %i", t);
	
#ifdef BEETEST_RANDOM
	xsrand64_seed(0x5fe705974ddbe33fLL);
#endif
	
	for(uint64_t ui = ITERR; ui > 0; ui -= 2) {
#ifndef BEETEST_RANDOM
		testbeek[0] = ui;
#endif
		for(uint64_t uiui = ITERR; uiui > 0; uiui -= 2) {
#ifdef BEETEST_RANDOM
			testbeek[0] = xsrand64();
			testbeek[1] = xsrand64();
			value = xsrand64();
#else
			testbeek[1] = uiui;
			value = ui ^ uiui;
#endif

			tlog(5, "addcount %i, adding k %llu %llu, v %llu", count++, testbeek[0], testbeek[1], value);
			if(count == 236) {
				tset_verbosity(10);
			}	
			fflush(stdout);

			t = bpt_add(&testbee, testbeek, value);
			tset_verbosity(5);
			if(t != 0) {
				yamc_terminate(-665, "error adding");
			}
			tlog(6, "wft %llu %llu", testbeek[0], testbeek[1]);
		}
	}
	tlog(5, "Bee insert done. took %li ms", SDL_GetTicks() - testticks);
	testticks = SDL_GetTicks();

	count = 1;
#ifdef BEETEST_RANDOM
	xsrand64_seed(0x5fe705974ddbe33fLL);
#endif

	for(uint64_t ui = ITERR; ui > 0; ui -= 2) {
#ifndef BEETEST_RANDOM
		testbeek[0] = ui;
#endif
		for(uint64_t uiui = ITERR; uiui > 0; uiui -= 2) {
#ifdef BEETEST_RANDOM
			testbeek[0] = xsrand64();
			testbeek[1] = xsrand64();
			value = xsrand64();
#else
			testbeek[1] = uiui;
			value = ui ^ uiui
#endif
			tlog(5, "rereading %i k %llu %llu, v %llu", count++, testbeek[0], testbeek[1], value);

			v = bpt_get(&testbee, testbeek);
			if(v != value) {
				terror("v %llu", v);
				yamc_terminate(-664, "error reading");
			}
#ifndef BEETEST_RANDOM
			testbeek[1] = uiui + 1;
			v = bpt_get(&testbee, testbeek);
			if(v != 0) {
				yamc_terminate(-663, "unallowed z key");	
			}
			testbeek[0] = ui + 1;
			v = bpt_get(&testbee, testbeek);
			if(v != 0) {
				yamc_terminate(-663, "unallowed xz key");	
			}
			testbeek[1] = uiui;
			v = bpt_get(&testbee, testbeek);
			if(v != 0) {
				yamc_terminate(-663, "unallowed x key");	
			}
			testbeek[0] = ui;
#endif
		}
	}
	tlog(5, "Bee check done. took %li ms", SDL_GetTicks() - testticks);

	beept_close(&testbee);
}*/
