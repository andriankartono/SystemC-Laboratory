// Stimulates the policer with a traffic pattern

#include <iomanip>
#include "data_gen.h"

void data_gen::generate() {

	unsigned short int iat;
	short unsigned int bytes;

	// at least the time needed for sending the previous packet
	iat = rand() % 10;

	dat.write(0);

	while(true) {
		wait(iat, SC_US);

		// Size values and time betweeen simulated packet arrivals are generated randomly
		// packet size is evenly distributed in the range from 64 to 1500 bytes (mean value = 782)
		bytes = (64 + rand() % 1436);


		iat = (bytes + rand() % (6045 - bytes)) / 155;

		dat.write(bytes);
		cout << std::setw(9) << sc_time_stamp() << ": '" << name()
				<< "'\tnumber of bytes: " << std::setw(4) << bytes << endl;
	}
}
