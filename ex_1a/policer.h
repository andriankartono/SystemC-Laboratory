#include "systemc.h"

SC_MODULE(policer) {
	// input for packet
	sc_in<short unsigned int> size;
	
	// output true or false depending on the result of the calculation
	sc_out<bool> discard;

	int r_pol;
	int max;
	int counter;
	sc_time last_arrival_time;
	int dec;

	void policer_proc();

	SC_CTOR(policer) {
		SC_METHOD(policer_proc);
		sensitive << size;
		dont_initialize();

		r_pol = 30;
		max = 1500;
		counter = 0;
		last_arrival_time = SC_ZERO_TIME;
	}
};
