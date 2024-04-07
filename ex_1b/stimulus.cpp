#include "stimulus.h"

//generate clock with a 10 ns period
void stimulus::c_gen() {

	bool takt = false;

	while(true) {
		takt = !takt;
		clk.write(takt);
		wait(5, SC_NS);
	}
}

// generate a reset signal rst_n
void stimulus::r_gen() {

	bool rst = false;
	rst_n.write(!rst);
	wait(17, SC_NS);
	rst = true;
	rst_n.write(!rst);
	wait(10, SC_NS);
	rst = false;
	rst_n.write(!rst);
	wait(SC_ZERO_TIME);
	// no while loop as we only reset once at the start
}
