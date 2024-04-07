#include "Cpu.h"
#include "IoModule.h"
#include "reporting.h"

using namespace sc_core;
using namespace tlm;

void Cpu::processor_thread(void) {

	while(true) {
		
		// read new packet descriptor

		// wait until a new packet arrive (first point in script)
		wait(packetReceived_interrupt.value_changed_event());

		std::vector<soc_address_t> list{OUTPUT_0_ADDRESS, OUTPUT_1_ADDRESS, OUTPUT_2_ADDRESS, OUTPUT_3_ADDRESS};
		soc_address_t port_address;

		while(packetReceived_interrupt.read()){
			// start the read transaction
			startTransaction(TLM_READ_COMMAND, PROCESSOR_QUEUE_ADDRESS, (unsigned char *) &m_packet_descriptor, sizeof(m_packet_descriptor)); //descriptor need to be casted to match payload type

		

		
		// Forward the packet descriptor to an arbitrary port

		port_address = list[rand() % list.size()]; // a random port taken
		startTransaction(TLM_WRITE_COMMAND, port_address, (unsigned char *) &m_packet_descriptor, sizeof(m_packet_descriptor));

		
		}
	}
}


// startTransaction

void Cpu::startTransaction(tlm_command command, soc_address_t address, unsigned char *data, unsigned int dataSize){
	
	// second point in script

	tlm_phase phase;
	sc_time delay_time;

	while(true)
	{
		delay_time = SC_ZERO_TIME;
		phase = BEGIN_REQ;
		tlm_sync_enum tlm_resp;

		// set values of payload
		payload.set_command(command);
		payload.set_address(address);
		payload.set_data_ptr(data);
		payload.set_data_length(dataSize);

		// set values of phase and delay
		tlm_resp = initiator_socket-> nb_transport_fw(payload, phase, delay_time);

		if(do_logging){
			cout<< sc_time_stamp()<<" "<< name() <<": nb_transport_fw called ";
		}

		wait(delay_time);

		// check for correct return value and transaction phase
		if(tlm_resp != TLM_UPDATED || phase != END_REQ){
			cerr << sc_time_stamp() << " Cpu::startTransaction transaction phase or return value not correct" <<  endl;
			exit(1);
		}

		if(do_logging){
			cout<< sc_time_stamp()<<" "<< name() <<": tlm_resp and phase updated (End of Request) ";
		}

		// wait until 2nd phase of transaction is finished
		// triggered by backward function nb_transport_bw

		wait(transactionFinished_event);

		if(payload.get_response_status() == TLM_OK_RESPONSE){
			break;
		}
	}
}



// nb_transport_bw: implementation of the backward path callback
tlm_sync_enum Cpu::nb_transport_bw(tlm_generic_payload& transaction,
		tlm_phase& phase, sc_time& delay_time) {
	

	if(phase != BEGIN_RESP) {
		cerr << sc_time_stamp()<< "expected BEGIN_RESP but not received" << endl;
		exit(1);
	}

	delay_time += sc_time(CLK_CYCLE_BUS); // make the bus transfer take one bus cycle
	transactionFinished_event.notify(delay_time);
	phase = END_RESP;
	return TLM_COMPLETED;

	
}

unsigned int Cpu::instances = 0;
