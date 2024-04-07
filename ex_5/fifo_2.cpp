#include <iostream>
#include <iomanip>
#include "fifo_2.h"

fifo_2::fifo_2(sc_module_name name, unsigned int fifo_size) : fifo_size(fifo_size) {
	// initialization of member variables/ports
	fifo_data = new unsigned char[fifo_size];
	wr_ptr = 0;
	rd_ptr = 0;
	fill_level = 0;
}

bool fifo_2::write_fifo(unsigned char *data, unsigned int &count) {
	bool result = false;

	unsigned int len; // amount of data written
	unsigned char *ptr; // pointer where to put data
	ptr = data;
	sc_time delay;

	if(fill_level + (int)count > fifo_size) // not enough space for all data
		len = fifo_size - fill_level; 		// none or less data will be written
	else {
		len = count;
		result = true;
	}

	
	// complete process

	// len is the number of value to be written before we reach the maximum index
	// write first and then delay or delay first -> follow reference

	wait(100*len, SC_NS);

	// Check if the index will exceed the array boundaries
	// It is more efficient to use memcpy but simpler to generate message
	//cout << std::setw(9) << sc_time_stamp() << ": '" << name() << "' "<< len << " words have been written: 0x " << hex;

	for(unsigned int i=0; i < len; i++){
		//std::cout << std::setw(2) << std::setfill('0') << static_cast<unsigned>(*(ptr+i)) << " ";
		fifo_data[wr_ptr] = *(ptr + i);
		wr_ptr = (wr_ptr + 1) % fifo_size;
		fill_level++;
	}

	//cout<<dec<<endl; // set the output back to decimals and end the line

	count = len; // Update value of count

	/*  
	Memcpy implementaion (not tested). 
	Commented out because generating messages are simpler in for loops
	
	unsigned int total_index = wr_ptr + len;
	int first_group_size = fifo_size - wr_ptr;
	int second_group_size = 0;
	
	memcpy(fifo_data + wr_ptr, data, first_group_size);
	count += first_group_size;
	wr_ptr += first_group_size;

	if(total_index > fifo_size){ // not equal because we include the number wr_ptr is pointing to
		second_group_size = len - first_group_size;
		memcpy(fifo_data, data + first_group_size, second_group_size); 
		count += second_group_size;
		wr_ptr = second_group_size;
	} */ 

	
	// if(fifo_size <= 50)
	// 	output_fifo_status();
	return result;
}

bool fifo_2::read_fifo(unsigned char *data, unsigned int &count) {
	bool result = false;

	unsigned int len;	// amount of data read
	unsigned char *ptr;	// pointer where to put data
	ptr = data;
	sc_time delay;

	if(fill_level < count)	// not enough data to read
		len = fill_level;	// none or less data will be read
	else {
		len = count;
		result = true;
	}

	
	// complete process

	wait(100 * len, SC_NS);

	// Cout location is a bit weird here because theoretically read is not done yet
	// There is a specific cout output in case of the fifo being empty -> not implemented
	// cout << std::setw(9) << sc_time_stamp() << ": '" << name() << "' "<< len << " words have been read: 0x " << hex;

	for (unsigned int i = 0; i < len ; i++){
		// cout << std::setw(2) << std::setfill('0') << static_cast<unsigned>(*(fifo_data + rd_ptr)) << " ";
		// Since data is not an array, we cant use array indexing here
		*(ptr + i) = fifo_data[rd_ptr];
		rd_ptr = (rd_ptr + 1) % fifo_size;
		fill_level--;
	}

	// cout<< dec << endl;

	count = len;


	
	// if(fifo_size <= 50)
	// 	output_fifo_status();
	return result;
}

// helper function to output content of FIFO
void fifo_2::output_fifo_status() {
	cout << "\tCurrent status of '" << name() << "': write address: "
			<< wr_ptr << ", read address: " << rd_ptr
			<< ", fill level: " << fill_level << endl;
	cout << "\t";
	cout << hex; // switch to hexadecimal mode;
	if(fill_level == 0) {
		for(unsigned int i = 0; i < fifo_size; i++)
			cout << "-- ";
	}
	else if(fill_level == fifo_size) {
		for(unsigned int i = 0; i < fifo_size; i++)
			cout << std::setw(2) << std::setfill('0') << (int)*(fifo_data + i)
					<< " ";
	}
	else if(wr_ptr > rd_ptr) {
		for(unsigned int i = 0; i < fifo_size; i++) {
			if((i >= rd_ptr) && (i < wr_ptr)) {
				cout << std::setw(2) << std::setfill('0')
						<< (int)*(fifo_data + i) << " ";
			}
			else
				cout << "-- ";
		}
	}
	else if(wr_ptr < rd_ptr) {
		for(unsigned int i = 0; i < fifo_size; i++) {
			if((i >= rd_ptr) || (i < wr_ptr)) {
				cout << std::setw(2) << std::setfill('0')
						<< (int)*(fifo_data + i) << " ";
			}
			else
				cout << "-- ";
		}
	}
	cout << dec << std::setfill(' ') << endl << endl;
}
