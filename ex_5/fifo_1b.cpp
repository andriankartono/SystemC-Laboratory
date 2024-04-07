#include <iomanip>
#include "fifo_1b.h"

fifo_1b::fifo_1b(sc_module_name name, unsigned int fifo_size) : fifo_size(fifo_size) {
	
	
	// register process
	SC_METHOD(read_write_fifo);
	sensitive << clk.pos();

	

	// initialization of member variables/ports
	fifo_data = new unsigned char[fifo_size];
	rd_ptr.write(0);
	wr_ptr.write(0);
	fill_level = 0;

	
	// initialize output ports
	valid.initialize(false);

	
}

void fifo_1b::read_write_fifo() {
	

	unsigned int rd_val = rd_ptr.read();
	unsigned int wr_val = wr_ptr.read();

	bool write = wr_en.read() && !full.read(); // Flag to check if we should write
	// Can be replaced with == 0 depending on fill level definition: 
	// here when we directly output to d_out, we consider this as fill_level = 1 and maximum fill_level = 6
	// if == 0 then fill_level only increase and decrease when update_out and write_through is true
	bool write_through = (fill_level < 1)  && !valid.read(); // Flag to check if we should directly output to d_out or to buffer

	// write first then read -> because if empty write directly to d_out
	// This saves one clock cycle
	if(write){
		unsigned char input = d_in.read();
		// write directly to output
		if(write_through){
			d_out.write(input); // as d_out is also used as storage, we have effectively fillsize+1
			valid.write(true);

			//cout << std::setw(9) << sc_time_stamp() << ": " << "New data available at d_out = " << static_cast<unsigned>(input) << std::endl;
		}
		else{
			unsigned int new_wr_val = (wr_val + 1) % fifo_size;

			fifo_data[wr_val] =  input;
			//cout << std::setw(9) << sc_time_stamp() << ": " << "FIFO queue new value = " << static_cast<unsigned>(input) << " at index " << wr_val << std::endl;
			
			wr_ptr.write(new_wr_val);
			//cout << std::setw(9) << sc_time_stamp() << ": " << "wr_ptr new value = " << new_wr_val << std::endl;
		}

		fill_level++;
	}

	bool read = rd_en.read() && valid.read(); // Flag to check if we should read
	bool update_out = (fill_level > 1);// Flag to check if d_out should be updated (based on rd_en)

	if(read){
		// consumer is reading the data and FIFO can output next word.		
		if(update_out){
			unsigned char output = fifo_data[rd_val];
			unsigned int new_rd_val = (rd_val + 1) % fifo_size;

			d_out.write(output);
			//cout << std::setw(9) << sc_time_stamp() << ": " << "FIFO d_out new value = " << static_cast<unsigned>(output) <<  " read from @ index " << rd_val << std::endl;
			rd_ptr.write(new_rd_val);
			//cout << std::setw(9) << sc_time_stamp() << ": " << "FIFO rd_ptr new value = " << new_rd_val << std::endl;
		}

		fill_level--;
		valid.write(update_out);
	}

	full.write(!(fill_level < (fifo_size + 1)));

	
}
