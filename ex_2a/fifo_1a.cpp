#include <iomanip>
#include "fifo_1a.h"

// FIFO takes value with subsequent clock cycles and stores it internally
// Pointers in FIFO: if both read and write pointer to the same thing -> empty
// Write pointer points to the next unused FIFO memory
// When an item is read from the buffer, the read pointer is incremented. (pointer references next item to be read)

fifo_1a::fifo_1a(sc_module_name name, unsigned int fifo_size) : fifo_size(fifo_size) {
	// register processes

	SC_METHOD(write_fifo);
	sensitive << clk.pos();

	SC_METHOD(read_fifo);
	sensitive << clk.pos();

	SC_METHOD(set_flags);
	sensitive << rd_ptr << wr_ptr;

	// initialization of member variables/ports
	fifo_data = new unsigned char[fifo_size];
	rd_ptr.write(0);
	wr_ptr.write(0);
	fill_level = 0;

	// initialize output ports
	
	full.initialize(false);
	empty.initialize(false);
	d_out.initialize(0);
}

void fifo_1a::write_fifo() {
	// Since wr_ptr is defined as integer, we assume wr_ptr is the offset (not the usual pointer)	
	// If the offset is larger than FIFO_size -> return to fill the first element in array of fifo_data
	// This is possible because of the full flag protecting us from overwriting unread elements of the queue

	// For future reference: Pointer arithmetic
	// adding an integer to a pointer will make the memory value increment to the next 4 size of pointer type instead of changing memory by the integer
	if (wr_en.read() && !full.read()){
		unsigned char input = d_in.read();
		unsigned int old_wr_value = wr_ptr.read();
		unsigned int new_wr_value = (old_wr_value + 1) % fifo_size;

		fifo_data[old_wr_value] =  input;
		cout << std::setw(9) << sc_time_stamp() << ": " << "FIFO queue new value = " << static_cast<unsigned>(input) << std::endl;
		fill_level++;
		wr_ptr.write(new_wr_value);
		cout << std::setw(9) << sc_time_stamp() << ": " << "wr_ptr new value = " << new_wr_value << std::endl;
	}
}

void fifo_1a::read_fifo() {
	if (rd_en.read() && !empty.read()){
		unsigned int old_rd_value = rd_ptr.read();
		unsigned int new_rd_value = (old_rd_value + 1) % fifo_size;

		d_out.write(fifo_data[old_rd_value]);
		cout << std::setw(9) << sc_time_stamp() << ": " << "FIFO d_out new value = " << static_cast<unsigned>(fifo_data[old_rd_value]) << std::endl;
		fill_level--;
		rd_ptr.write(new_rd_value);
		cout << std::setw(9) << sc_time_stamp() << ": " << "FIFO rd_ptr new value = " << new_rd_value << std::endl;
	}
}

void fifo_1a::set_flags() {
	// set 'full' & 'empty' flags according to fill level

	full.write(fill_level == fifo_size);
	empty.write(fill_level == 0);
}
