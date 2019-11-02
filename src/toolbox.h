#ifndef __TOOLBOX_H_INCLUDED__
#define __TOOLBOX_H_INCLUDED__

#include <errno.h>

#include "plogger.h"


#define yamc_terminate(ERROR, MSG) \
{ \
	terror("Game got terminated from file \"%s\" line %i. Reason:\n%s\n", \
			__FILE__, __LINE__, MSG); \
	exit(ERROR); \
}

void xsrand64_seed(uint64_t);
uint64_t xsrand64(void);


#endif
