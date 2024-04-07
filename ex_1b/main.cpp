#include "stimulus.h"
#include "counter.h"
#include "bcd_decoder.h"

int sc_main(int argc, char *argv[]) {

	sc_signal<bool> clock, reset_n;
	sc_signal<unsigned short int> count_val;
	sc_signal<char> v_hi, v_lo;

	// instantiate and connect counter module here
	stimulus stim1("stimulus");
	bcd_decoder bcd_deco1("bcd_decoder");
	counter counter1("counter");

	//connect signals to port 
	stim1.clk(clock);
	stim1.rst_n(reset_n);
	
	counter1.clk(clock);
	counter1.rst_n(reset_n);
	counter1.cnt(count_val);

	bcd_deco1.val(count_val);
	bcd_deco1.hi(v_hi);
	bcd_deco1.lo(v_lo);

	// Create trace file and record relevant traces
	sc_trace_file *tf = sc_create_vcd_trace_file("traces");
	sc_trace(tf, clock, "clock");
	sc_trace(tf, count_val, "count_val");
	sc_trace(tf, reset_n, "reset_n");
	sc_trace(tf, v_hi, "v_hi");
	sc_trace(tf, v_lo, "v_lo");

	int n_cycles;
	if(argc != 2) {
		cout << "default n_cycles = 2000" << endl;
		n_cycles = 2000;
	}
	else {
		n_cycles = atoi(argv[1]);
		cout << "n_cycles = " << n_cycles << endl;
	}

	sc_start(n_cycles, SC_NS);

	sc_close_vcd_trace_file(tf);

	return 0;
}
