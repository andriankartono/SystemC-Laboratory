#include "consumer.h"
#include "producer.h"
#include "fifo_1b.h"

int sc_main(int argc, char *argv[]) {
	// the following instruction generates a clock signal with clock named
	// "clock" with a period of 100 ns, a duty cycle of 50%, and a falling edge
	// after 50 ns
	sc_clock clk("clock", 100, SC_NS, 0.5, 50, SC_NS, false);

	// fill in the required commands to instantiate and connect producer, fifo,
	// and consumer

	// signals related to producer
	sc_signal<bool> wr_en;
	sc_signal<unsigned char> d_in;
	sc_signal<bool> full;

	// signals related to consumer
	sc_signal<bool> rd_en;
	sc_signal<unsigned char> d_out;
	sc_signal<bool> valid;

	// Initialize modules
	consumer consum_inst("consumer");
	producer prod_inst("producer");
	fifo_1b fifo_inst1b("fifo");

	// Connect ports to the modules
	consum_inst.clk(clk);
	consum_inst.rd_en(rd_en);
	consum_inst.valid(valid);
	consum_inst.d_in(d_out);

	prod_inst.clk(clk);
	prod_inst.wr_en(wr_en);
	prod_inst.full(full);
	prod_inst.d_out(d_in);

	fifo_inst1b.clk(clk);
	fifo_inst1b.d_in(d_in);
	fifo_inst1b.d_out(d_out);
	fifo_inst1b.valid(valid);
	fifo_inst1b.full(full);
	fifo_inst1b.wr_en(wr_en);
	fifo_inst1b.rd_en(rd_en);

	// fill in code to generate traces that can be used to observe the
	// functionality of the model with the waveform viewer gtkwave

	sc_trace_file *tf = sc_create_vcd_trace_file("traces");

	sc_trace(tf, clk, "clk");
	sc_trace(tf, fifo_inst1b.fill_level, "fifo_fill_level");
	sc_trace(tf, full, "fifo_full");
	sc_trace(tf, d_in, "fifo_in");
	sc_trace(tf, d_out, "fifo_out");
	sc_trace(tf, valid, "fifo_valid");
	sc_trace(tf, rd_en, "rd_en");
	sc_trace(tf, wr_en, "wr_en");

	sc_time sim_dur = sc_time(5000, SC_NS);
	if(argc != 2) {
		cout << "Default simulation time = " << sim_dur << endl;
	}
	else {
		sim_dur = sc_time(atoi(argv[1]), SC_NS);
		cout << "Simulation time = " << sim_dur << endl;
	}

	// start simulation
	sc_start(sim_dur);

	sc_close_vcd_trace_file(tf);

	return 0;
}
