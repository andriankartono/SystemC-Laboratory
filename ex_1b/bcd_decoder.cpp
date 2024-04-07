#include "bcd_decoder.h"

void bcd_decoder::decode() {
	// converts counter value to BCD
	// output v_lo: char
	// output v_hi: char

	unsigned short int input_value = val.read();
	if (input_value > 99 || input_value < 0){
		//raise error
		cout<<"unallowed value detected while converting. Value = "<< input_value; 
	}

	// This implementation follows the lecture
	int b = input_value%10;
	lo.write(char(input_value%10));
	hi.write(char(int(((input_value-b)%100)/10)));
}
