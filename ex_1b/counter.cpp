#include "counter.h"

void counter::count() {
	// synchronously (with rising edge of clock) count from 0 to 99 (overflow -> reset to 0)
	// should be possible to reset counter asynchronously (active low: counter reset to 0 when rst_n==0 )
	// output type = unsigned short int

	while(true) {
		wait(); //should wait be set to rising edge here for safety?
		
		// increase counter
		if (cnt_int>=99){
			cnt_int = 0;
		}else{
			cnt_int++;
		}

		if(rst_n==0){
			cnt_int = 0;
		}

		cnt.write(cnt_int);
	}
}
