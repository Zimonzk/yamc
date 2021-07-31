#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include "../src/plogger.h"

#include "test_beetree.h"


int main() {

	tset_verbosity(5);
	tlog(5, "Verbosity set to %i.", global_verbosity);

	test_beetree();

	return 0;
}
