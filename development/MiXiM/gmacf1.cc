/* G-MAC, fixed-sequence, neighbours with same slot assignment can't
   communicate.
*/

#include "gmacf1.h"

Define_Module_Like( GMacF1, MacClass );

int GMacF1::slots;
bool GMacF1::parametersInitialised = false;

void GMacF1::initialize() {
	/* Max header contains from, type, to, and clock data */
	max_header_length = ADDRESS_BYTES + TYPE_BYTES + ADDRESS_BYTES + CLOCK_BYTES;
	/* Min header contains from, type, and clock data */
	min_header_length = ADDRESS_BYTES + TYPE_BYTES + CLOCK_BYTES;
	
	GMac::initialize();

	if (!parametersInitialised) {
		parametersInitialised = true;
		slots = getLongParameter("slots", 16);
	}
}

void GMacF1::finish() {
	GMac::finish();
}

GMacF1::~GMacF1() {
	parametersInitialised = false;
}

void GMacF1::txPacket(Packet * msg) {
	GMac::txPacket(msg);
	
	if ((msg->local_to % slots) == (node->getNodeId() % slots)) {
		printf(PRINT_ROUTING, "Can't send message, dropping");
		msg->setKind(TX_FAILED);
		txPacketDone(msg);
		++stat_tx_drop;
		tx_msg = NULL;
		return;
	}
}

void GMacF1::wrapSlotCounter() {
	/* Default implementation for wrapSlotCounter. */
	if (current_slot == slots + 1)
		current_slot = 0;
}

GMac::SlotState GMacF1::getCurrentSlotState() {
	if (current_slot == (node->getNodeId() % slots)) {
		/* Listening in this slot. */
		printf(PRINT_MAC, "listening");
		return SSTATE_RECEIVE;
	} 

	if (tx_msg) {
		if (tx_msg->local_to == BROADCAST) {
			if (current_slot == slots)
				return SSTATE_SEND_RECEIVE;
#ifdef SINK_ALWAYS_ON // Assumes that node 0 is sink!
		} else if ((tx_msg->local_to % slots) == current_slot || (tx_msg->local_to == 0 && current_slot != slots)) {
#else
		} else if ((tx_msg->local_to % slots) == current_slot) {
#endif
			/* Unicast message for a node awake in this slot. */
			return SSTATE_SEND;
		}
	}

	if (current_slot == slots)
		return SSTATE_RECEIVE;

#ifdef SINK_ALWAYS_ON // Assumes that node 0 is sink!
	if (node->getNodeId() == 0)
		return SSTATE_RECEIVE;
#endif

	return SSTATE_SLEEP;
}

int GMacF1::slotsUntilWake(int destination) {
	int destinationSlot = destination % slots;
	
	if (destinationSlot == node->getNodeId() % slots)
		return INT_MAX;
	
	destinationSlot -= current_slot;
	if (destinationSlot <= 0)
		destinationSlot += slots + 1;
	return destinationSlot;
}
