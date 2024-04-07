#include "systemc.h"

SC_MODULE(data_gen) {
	// Output port with size of arriving packet
	// new value -> new packet arrival
	sc_out<short unsigned int> dat;

	void generate();

	SC_CTOR(data_gen) {
		SC_THREAD(generate);
		srand(123);
	}
};
