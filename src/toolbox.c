#include "toolbox.h"

static uint64_t xs64state;

void xsrand64_seed(uint64_t seed)
{
	xs64state = seed;
}

uint64_t xsrand64(void)
{
	register uint64_t r = xs64state;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;

	xs64state = r;

	return r;
}
