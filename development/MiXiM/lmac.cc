/*
 * LMAC
 *
 * A simple TDMA mac protocol.
 * We cheat on the time synchronisation part, by having all nodes be in sync
 * from the start. However, if they haven't heard a neighbour they will
 * listen as if they are unsynchronised.
 */


#include "mixim.h"
#include "message.h"

#include "lmac.h"

Define_Module_Like( LMac, MacClass );

#define CLOCK_SKEW_ALLOWANCE 2
/*Note: the length is already encoded in the FRAME_TOTAL_TIME macro
 thus the header is only 11 bytes */
#define SLOT_TIME	(int)((frameTotalTime(header_length+data_length)+EXTRA_TRANSMIT_TIME)*32768.0+CLOCK_SKEW_ALLOWANCE)
#define SHORT_WAIT	(int)(CLOCK_SKEW_ALLOWANCE+3.0)

enum {
	SLOT_TIMER,
	SHORT_TIMEOUT,
	SEND_DELAY,
	HEADER_TIMEOUT
};

#define SET(a,b) do { (a)[(b)/64] |= ((uint64_t) 1) << ((b) % 64); } while(0)
#define UNSET(a,b) do { (a)[(b)/64] &= UINT64_MAX ^ (((uint64_t) 1) << ((b) % 64)); } while(0)
#define ISSET(a,b) ((a)[(b)/64] & (((uint64_t) 1) << ((b) % 64)))
#define MERGE(a,b) do { int _i; for (_i = 0; _i < SLOT_WORDS; _i++) (a)[_i] |= (b)[_i]; } while(0)

void LMac::initialize() {
	Mac::initialize();
	printfNoInfo(PRINT_INIT, "\t\tLMAC initializing...");

	tx_msg = NULL;
	current_slot = -1;
	my_slot = -1;
	no_slots = 0;
	backoff = 0;
	initialized = false;
	memset(collision_slots, 0, sizeof(collision_slots));
	memset(tentative_slots, 0, sizeof(tentative_slots));
	max_slot = getLongParameter("maxSlot", 32);
	if (max_slot > MAX_SLOTS || max_slot < 1)
       ev.printf("maxSlot value out of range (1..%d). Increase MAX_SLOTS in lmac.h and recompile if more slots are required.\n", MAX_SLOTS);
	
	header_length = 7 + (max_slot + 7) / 8;

	//data_length = getLongParameter("dataLength", 64);
	/*101 bytes makes frame length 1.595 seconds, which corresponds
	 *to current (50ms * 32slots) 1.6 second frames */
	data_length = getLongParameter("dataLength", 101);
	use_tentative = getBoolParameter("useTentative", false);

	ignore_failed = false;
	if (node->getNodeId() == 0) {
		initialized = true;
		my_slot = 0;
		distance_to_gateway = 0;
	}
	for (int i = 0; i < max_slot; i++) {
		neighbour_info[i].id = -1;
		memset(neighbour_info[i].occupied_slots, 0, sizeof(neighbour_info[i].occupied_slots));
	}
	sendingData = false;
	verify_slot = false;

	setTimeout(2, SLOT_TIMER);
	setRadioListen();
}

void LMac::finish() {
	Mac::finish();
	printfNoInfo(PRINT_INIT, "\t\tLMAC ending...");
	if (tx_msg)
		delete tx_msg;
}

void LMac::txPacket(Packet * msg){
	assert(msg);
	if(tx_msg) {
		printf(PRINT_ROUTING, "MAC busy! dropping at tx_packet");
		++stat_tx_drop;
		delete msg;
		return;
	}
	tx_msg = msg;
}

void LMac::rxFrame(Packet * msg) {
	assert(msg);
	if(msg->local_to == node->getNodeId()) {
		printf(PRINT_MAC, "unicast frame received");	
		reg_rx_data(msg);
		rxPacket(msg);
		++stat_rx;
	} else if(msg->local_to == BROADCAST) {
		printf(PRINT_MAC, "local broadcast received");
		reg_rx_data(msg);
		rxPacket(msg);
		++stat_rx;
	} else {
		printf(PRINT_MAC, "overheard frame, not for me");
		reg_rx_overhear(msg);
		delete msg;
	}
	ignore_failed = false;
	setRadioSleep();
}

void LMac::rxHeader(Packet * msg) {
	assert(msg);
	Header *header;
	
	//~ printf(PRINT_MAC, "received header from %d", msg->local_from);
	header = (Header *) msg->getData(MAC_DATA);
	if (!initialized) {
		current_slot = header->current_slot;
		distance_to_gateway = header->distance_to_gateway + 1;
		initialized = true;
	} else if (my_slot >= 0 && header->collision_slot == my_slot && node->getNodeId() != 0) {
		/* Someone is telling me I have collided with someone else. For node 0
		   we ignore this and just keep our slot. This is to prevent the
		   basestation from ending up without a slot. */
		my_slot = -1;
		memset(collision_slots, 0, sizeof(collision_slots));
		backoff = (int) intuniform(node->getNodeId() % max_slot, max_slot * 2, RNG_MAC);
		printf(PRINT_MAC, "collision in my slot! [backing off for %d]", backoff);
	} else if (header->collision_slot >= 0) {
		UNSET(collision_slots, header->collision_slot);
		neighbour_info[header->collision_slot].id = -1;
		memset(neighbour_info[header->collision_slot].occupied_slots, 0, sizeof(neighbour_info[header->collision_slot].occupied_slots));
	}

	neighbour_info[current_slot].id = msg->local_from;
	memcpy(neighbour_info[current_slot].occupied_slots, header->occupied_slots, sizeof(neighbour_info[current_slot].occupied_slots));

	if (distance_to_gateway > header->distance_to_gateway + 1)
		distance_to_gateway = header->distance_to_gateway + 1;

	if (my_slot < 0 && backoff == 0)
		pickSlot();
	
	if (msg->local_to != node->getNodeId() && msg->local_to != BROADCAST) {
		//~ printf(PRINT_MAC, "Packet not meant for me but for node %d", msg->local_to);
		setRadioSleep();
		ignore_failed = true;
	}
}

void LMac::rxFailed() {
	if (!ignore_failed && initialized) {
		//~ printf(PRINT_MAC, "Reception failed!!!!!");
		SET(collision_slots, current_slot);
		neighbour_info[current_slot].id = -1;
		memset(neighbour_info[current_slot].occupied_slots, 0, sizeof(neighbour_info[current_slot].occupied_slots));
		setRadioSleep();
	}
	ignore_failed = false;
}

void LMac::rxStarted() {
	//~ setTimeout((int)ceil(FRAME_DATA_TIME(header_length)+30), HEADER_TIMEOUT);
	cancelTimeout(HEADER_TIMEOUT);
}

void LMac::transmitDone() {
	//~ printf(PRINT_MAC, "transmit complete");
	// cleanup
	if (tx_msg && sendingData) {
		tx_msg->setKind(TX_DONE);
		txPacketDone(tx_msg); // report success
		tx_msg = NULL;
		sendingData = false;

		++stat_tx;
	}
	setRadioSleep();
}

void LMac::timeout(int which) {
	
	switch(which) {
		case SLOT_TIMER:
			setTimeout(SLOT_TIME, SLOT_TIMER);
			current_slot++;
			current_slot %= max_slot;

			if (use_tentative)
				UNSET(tentative_slots, current_slot);

			UNSET(collision_slots, current_slot);

			if (backoff > 0 && --backoff == 0)
				pickSlot();
			if (my_slot == current_slot) {
				if (verify_slot) {
					verify_slot = false;
					if (neighbour_info[current_slot].id >= 0) {
						printf(PRINT_MAC, "other node got there first, choosing new slot");
						setTimeout(SHORT_WAIT, SHORT_TIMEOUT);
						setRadioListen();
						my_slot = -1;
						backoff = intuniform(max_slot, max_slot * 2, RNG_MAC);
						return;
					}
					for (int i = 0; i < max_slot; i++) {
						if (neighbour_info[i].id >= 0 && ISSET(neighbour_info[i].occupied_slots, current_slot)) {
							printf(PRINT_MAC, "other node got there first, choosing new slot");
							setTimeout(SHORT_WAIT, SHORT_TIMEOUT);
							setRadioListen();
							my_slot = -1;
							backoff = intuniform(max_slot, max_slot * 2, RNG_MAC);
							return;
						}
					}
				}
				
				printf(PRINT_MAC, "=== Slot %d (owner %d) ===", current_slot, node->getNodeId());
				//~ printf(PRINT_MAC, "My slot %d", current_slot);
				setTimeout(CLOCK_SKEW_ALLOWANCE, SEND_DELAY);
			} else {
				//~ printf(PRINT_MAC, "Not my slot %d", current_slot);
				if (initialized)
					setTimeout(SHORT_WAIT, SHORT_TIMEOUT);
				setRadioListen();
			}
			break;
		case SHORT_TIMEOUT:
			//check for transmission
			if (getRssi() < 0.5) {
				//slot is free
				//~ printf(PRINT_MAC, "channel clear");
				neighbour_info[current_slot].id = -1;
				memset(neighbour_info[current_slot].occupied_slots, 0, sizeof(neighbour_info[current_slot].occupied_slots));
				setRadioSleep();
			} else if (getRssi() < 0.6) {
				//slot is free, but used far away
				//~ printf(PRINT_MAC, "channel clear");
				if (use_tentative)
					SET(tentative_slots, current_slot);
				setRadioSleep();
			} else {
				//someone is sending (but may be collision)
				//~ printf(PRINT_MAC, "channel busy");
				setTimeout((int)(frameTotalTime(0) * 32768.0), HEADER_TIMEOUT);
			}
			return;
		case SEND_DELAY: {
			Packet *current_msg;
			Header header;
			
			assert(my_slot >= 0);

			header.current_slot = current_slot;
			memset(header.occupied_slots, 0, sizeof(header.occupied_slots));
			for (int i = 0; i < max_slot; i++)
				if (neighbour_info[i].id >= 0)
					SET(header.occupied_slots, i);
			SET(header.occupied_slots, my_slot);
			if (use_tentative)
				MERGE(header.occupied_slots, tentative_slots);
			header.distance_to_gateway = distance_to_gateway;
			header.collision_slot = -1;
			if (node->getNodeId() == 0) {
				int i;
				for (i = 0; i < SLOT_WORDS; i++)
					printf(PRINT_MAC, "Collision slots: 0x%016" PRIX64, collision_slots[i]);
			}
			for (int i = 0; i < max_slot; i++) {
				if (ISSET(collision_slots, i)) {
					header.collision_slot = i;
					UNSET(collision_slots, i);
					break;
				}
			}

			//now do real send
			//~ printf(PRINT_MAC, "Starting transmit");
			setRadioTransmit();
			if (tx_msg) {
				sendingData = true;
				reg_tx_data(tx_msg); // statistics
				current_msg = (Packet *)tx_msg->dup();;
				printf(PRINT_MAC, "Sending message to %d", tx_msg->local_to);
			} else {
				char buffer[100];
				sprintf(buffer, "header only %d", node->getNodeId());
				//~ current_msg = new Packet("header only");
				current_msg = new Packet(buffer);
				current_msg->setLength(0); //only header
				current_msg->setLocalTo(-2); 	// no target because we don't have data
				reg_tx_overhead(current_msg);
			}
			current_msg->setData(MAC_DATA, &header, sizeof(header), 0);
			current_msg->local_from = node->getNodeId();
			startTransmit(current_msg);
			return;
		}
		case HEADER_TIMEOUT: {
			//~ printf(PRINT_MAC, "Should have received header by now");
			ignore_failed = true;
			SET(collision_slots, current_slot);
			neighbour_info[current_slot].id = -1;
			memset(neighbour_info[current_slot].occupied_slots, 0, sizeof(neighbour_info[current_slot].occupied_slots));
			setRadioSleep();
			break;
		}
		default:
			assert(false); // illegal state
	}
}

int LMac::headerLength() {
	// LMac has 12 byte header containing, but
	// length is already coded elsewhere
	return header_length;
}

void LMac::pickSlot() {
	uint64_t occupied_slots[SLOT_WORDS];
	int i, unoccupied = 0;
	
	memset(occupied_slots, 0, sizeof(occupied_slots));

	if (no_slots > 10) {
/*		cancelTimeout(SLOT_TIMER);
		cancelTimeout(SHORT_TIMEOUT);
		setRadioSleep();
		printf(PRINT_MAC, "No slots available, turning off");*/
		return;
	}

	for (i = 0; i < max_slot; i++)
		MERGE(occupied_slots, neighbour_info[i].occupied_slots);

	if (use_tentative)
		MERGE(occupied_slots, tentative_slots);
	{ int i;
		for (i = 0; i < SLOT_WORDS; i++)
			printf(PRINT_MAC, "Occupied: %" PRIX64, occupied_slots[i]);
	}

/* 	if ((occupied_slots & MASK) == MASK) {
		printf(PRINT_MAC, "No unoccupied slots in my neighbourhood :-(");
		no_slots++;
		return;
	} */
	no_slots = 0;

	for (i = 0; i < max_slot; i++)
		if (!(ISSET(occupied_slots, i)))
			unoccupied++;

	if (unoccupied == 0) {
		printf(PRINT_MAC, "No unoccupied slots in my neighbourhood :-(");
		no_slots++;
		return;
	}

	int slot = (int) intuniform(0, unoccupied - 1, RNG_MAC);
	unoccupied = 0;
	for (i = 0; i < max_slot; i++) {
		if (!(ISSET(occupied_slots, i))) {
			if (unoccupied == slot) {
				my_slot = i;
				verify_slot = true;
				printf(PRINT_MAC, "My slot %d", my_slot);
				break;
			}
			unoccupied++;
		}
	}
}
