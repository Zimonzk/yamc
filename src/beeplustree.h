#ifndef __BEEPLUSTREE_H_INCLUDED__
#define __BEEPLUSTREE_H_INCLUDED__

#define _FILE_OFFSET_BITS 64
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif


#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>

#define xz_greater(XZ1, XZ2) \
	((XZ1[0] > XZ2[0]) || ((XZ1[0] == XZ2[0]) && (XZ1[1] > XZ2[1])))

#define xz_equal(XZ1, XZ2) \
	((XZ1[0] == XZ2[0]) && (XZ1[1] == XZ2[1]))


struct bpt_node {
	uint8_t is_leaf;
	uint16_t nrval;
	uint64_t rvals[31][2];
	off64_t children[32];
};

struct beept {
	FILE *f;
	struct bpt_node curr_node;
};


int beept_init(struct beept *bpt, char *path);
void beept_close(struct beept *bpt);

uint64_t 	bpt_get(struct beept *bpt, uint64_t key[2]);
int 		bpt_add(struct beept *bpt, uint64_t key[2], uint64_t value);

/* yet to be implemented */
int		bpt_del(struct beept *bpt, uint64_t key[2]);


void beeplustest(void);

#endif
