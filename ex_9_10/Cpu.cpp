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

		while(packetReceived_interrupt.read()){
			// start the read transaction
			
            //read descriptor
            MEASURE_TRANSFER_TIME(startTransaction(TLM_READ_COMMAND, PROCESSOR_QUEUE_ADDRESS, (unsigned char *) &m_packet_descriptor, sizeof(m_packet_descriptor));) //descriptor need to be casted to match payload type

            //read header packet
            unsigned int sizetoread = sizeof(sc_time) + sizeof(uint64_t) + IpPacket::MINIMAL_IP_HEADER_LENGTH;
            MEASURE_TRANSFER_TIME(startTransaction(TLM_READ_COMMAND, m_packet_descriptor.baseAddress, (unsigned char *) &m_packet_header, sizetoread);)

			bool integrityverified = verifyHeaderIntegrity(m_packet_header);
			MEASURE_PROCESSING_TIME(
				wait(CPU_VERIFY_HEADER_CYCLES * CLK_CYCLE_CPU);
			)

            // Verify header
			// there are duplicated code because page 33 reccomends reading first and then header verification while beginning of section 2 say verify first
			// therefore the accelerator part and cpu part are separated completely

			if(use_accelerator){
				bool discard = false; // flag to see if the packet need to be discarded

				m_lookup_request.processorId = m_id;
				m_lookup_request.destAddress = m_packet_header.getDestAddress();
				// forward lookup_request to accelerator
				MEASURE_TRANSFER_TIME(startTransaction(TLM_WRITE_COMMAND, ACCELERATOR_ADDRESS, (unsigned char *) &m_lookup_request, sizeof(m_lookup_request));)

				if(integrityverified){
					int newTTL = decrementTTL(m_packet_header);
					MEASURE_PROCESSING_TIME(
						wait(CPU_DECREMENT_TTL_CYCLES * CLK_CYCLE_CPU);
					)
					
					if(newTTL == 0){
						//discard packet if TTL is 0
						discard = true;
					}
					else{

						// update checksum if packet is not to be dropped
						updateChecksum(m_packet_header);
						MEASURE_PROCESSING_TIME(
							wait(CPU_UPDATE_CHECKSUM_CYCLES * CLK_CYCLE_CPU);
						)

						//write header back to memory
						MEASURE_TRANSFER_TIME(startTransaction(TLM_WRITE_COMMAND, m_packet_descriptor.baseAddress, (unsigned char *) &m_packet_header, sizetoread);)
					}
				}
				else{
					discard = true;
				}

				if(!lookupReady_interrupt.read())
					wait(lookupReady_interrupt.value_changed_event());

				// read from accelerator
				unsigned int nextport;
				MEASURE_TRANSFER_TIME(startTransaction(TLM_READ_COMMAND, ACCELERATOR_ADDRESS,(unsigned char *) &nextport, sizeof(unsigned int));)

				soc_address_t target_address;
				if(discard == true){
					target_address = DISCARD_QUEUE_ADDRESS;
				}else{
					target_address = list[nextport];	
				}

				//forward packet descriptor to output
				MEASURE_TRANSFER_TIME(startTransaction(TLM_WRITE_COMMAND, target_address, (unsigned char *) &m_packet_descriptor, sizeof(m_packet_descriptor));)

			}
			else{
				// Use CPU instead of accelerator(slower)
				if(integrityverified){

					// get the port ID
					unsigned int nextport = makeNHLookup(m_packet_header);
					MEASURE_PROCESSING_TIME(
						wait(CPU_IP_LOOKUP_CYCLES * CLK_CYCLE_CPU);
					)


					// decrement TTL
					int newTTL = decrementTTL(m_packet_header);
					MEASURE_PROCESSING_TIME(
						wait(CPU_DECREMENT_TTL_CYCLES * CLK_CYCLE_CPU);
					)

					if(newTTL == 0){
						//discard packet if TTL is 0
						MEASURE_TRANSFER_TIME(startTransaction(TLM_WRITE_COMMAND, DISCARD_QUEUE_ADDRESS, (unsigned char *) &m_packet_descriptor, sizeof(m_packet_descriptor));)
					
					}else{

						// update checksum if packet is not to be dropped
						updateChecksum(m_packet_header);
						MEASURE_PROCESSING_TIME(
							wait(CPU_UPDATE_CHECKSUM_CYCLES * CLK_CYCLE_CPU);
						)

						//write header back to memory
						MEASURE_TRANSFER_TIME(startTransaction(TLM_WRITE_COMMAND, m_packet_descriptor.baseAddress, (unsigned char *) &m_packet_header, sizetoread);)

						//forward packet descriptor to output
						MEASURE_TRANSFER_TIME(startTransaction(TLM_WRITE_COMMAND, list[nextport], (unsigned char *) &m_packet_descriptor, sizeof(m_packet_descriptor));)
					}
				}else{

					// Discard header if failed -> write packet descriptor to discard queue (page 24)
					MEASURE_TRANSFER_TIME(startTransaction(TLM_WRITE_COMMAND, DISCARD_QUEUE_ADDRESS, (unsigned char *) &m_packet_descriptor, sizeof(m_packet_descriptor));)
				}
			}
		
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
