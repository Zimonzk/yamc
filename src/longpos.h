#ifndef __LONGPOS_H_INCLUDED__
#define __LONGPOS_H_INCLUDED__

#include "chunk.h"
#include <math.h>

struct longpos {
	long chunk [2]; /*x and z number of the chunk this position refers to*/	
	double rpos[3]; /*x, y and z coordinates of the position
			 *relative to the chunk*/
}

#define lpos_dist(P1, P2) \
	sqrt(pow((P2.chunk[0] - P1.chunk[0]) * CHUNK_LIM_HOR + \
		P2.rops[0] - P1.rpos[0], 2) + \
	     pow(P2.rpos[1] - P1.rpos[1], 2) + \
	     pow((P2.chunk[1] - P1.chunk[1]) * CHUNK_LIM_HOR + \
		P2.rops[2] - P1.rpos[2], 2, 2))


#endif /*__LONGPOS_H_INCLUDED__*/
