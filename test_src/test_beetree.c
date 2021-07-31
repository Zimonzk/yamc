#include "test_beetree.h"
#include <assert.h>

#include "../src/plogger.h"
#include "zimonzk/lists.h"
#include "../src/toolbox.h"
#include <stdio.h>

void test_beetree(void)
{
#define TEST_ITERATIONS 10000000
	struct beept testbee = {};

	/*ensure there is no old leftover tesst beeplustree.*/
	remove("test.beept");

	int t = beept_init(&testbee, "test.beept");

	uint64_t testkey[2];
	uint64_t testvalue;
	uint64_t retvalue;

	xsrand64_seed(0x5fe705974ddbe33fLL);

	tlog(5, "--- writing ---");

	for(int i = 0; i < TEST_ITERATIONS; i++) {
		testkey[0] = xsrand64();
		testkey[1] = xsrand64();
		testvalue = xsrand64();

		tlog(6, "%i k %llu %llu v %llu", i, testkey[0], testkey[1], testvalue);

		t = bpt_add(&testbee, testkey, testvalue);
		assert(t == 0);
	}

	xsrand64_seed(0x5fe705974ddbe33fLL);

	tlog(5, "--- rereading ---");

	for(int i = 0; i < TEST_ITERATIONS; i++) {
		testkey[0] = xsrand64();
		testkey[1] = xsrand64();
		testvalue = xsrand64();

		tlog(6, "%i k %llu %llu v %llu", i, testkey[0], testkey[1], testvalue);

		retvalue = bpt_get(&testbee, testkey);

		tset_verbosity(5);

		assert(retvalue == testvalue);
	}

	remove("test.beept");
}
