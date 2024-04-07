#include "consumer.h"
#include "producer.h"
#include "fifo_1a.h"

int sc_main(int argc, char *argv[]) {
	// the following instruction generates a clock signal with clock named
	// "clock" with a period of 100 ns, a duty cycle of 50%, and a falling edge
	// after 50 ns
	sc_clock clk("clock", 100, SC_NS, 0.5, 50, SC_NS, false);

	// ############# COMPLETE THE FOLLOWING SECTION ############# //
	// fill in the required commands to instantiate and connect producer, fifo,
	// and consumer

	// signals related to producer
	sc_signal<bool> wr_en;
	sc_signal<unsigned char> d_in;
	sc_signal<bool> full;

	// signals related to consumer
	sc_signal<bool> rd_en;
	sc_signal<unsigned char> d_out;
	sc_signal<bool> empty;

	// Initialize modules
	consumer consum_inst("consumer");
	producer prod_inst("producer");
	fifo_1a fifo_inst1a("fifo");

	// Connect ports to the modules
	consum_inst.clk(clk);
	consum_inst.rd_en(rd_en);
	consum_inst.empty(empty);
	consum_inst.d_in(d_out);

	prod_inst.clk(clk);
	prod_inst.wr_en(wr_en);
	prod_inst.full(full);
	prod_inst.d_out(d_in);

	fifo_inst1a.clk(clk);
	fifo_inst1a.d_in(d_in);
	fifo_inst1a.d_out(d_out);
	fifo_inst1a.empty(empty);
	fifo_inst1a.full(full);
	fifo_inst1a.wr_en(wr_en);
	fifo_inst1a.rd_en(rd_en);

	// ############# COMPLETE THE FOLLOWING SECTION ############# //
	// fill in code to generate traces that can be used to observe the
	// functionality of the model with the waveform viewer gtkwave

	sc_trace_file *tf = sc_create_vcd_trace_file("traces");

	sc_trace(tf, clk, "clk");
	sc_trace(tf, empty, "fifo_empty");
	sc_trace(tf, fifo_inst1a.fill_level, "fifo_fill_level");
	sc_trace(tf, full, "fifo_full");
	sc_trace(tf, d_in, "fifo_in");
	sc_trace(tf, d_out, "fifo_out");
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
