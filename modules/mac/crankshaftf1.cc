/* Crankshaft, fixed-sequence, neighbours with same slot assignment can't
   communicate.
*/

#include "crankshaftf1.h"

Define_Module_Like( CrankshaftF1, EyesMacLayer );

int CrankshaftF1::slots;
bool CrankshaftF1::parametersInitialised = false;

void CrankshaftF1::initialize() {
	/* Max header contains from, type, to, and clock data */
	max_header_length = ADDRESS_BYTES + TYPE_BYTES + ADDRESS_BYTES + CLOCK_BYTES;
	/* Min header contains from, type, and clock data */
	min_header_length = ADDRESS_BYTES + TYPE_BYTES + CLOCK_BYTES;
	
	CrankshaftBase::initialize();

	if (!parametersInitialised) {
		parametersInitialised = true;
		slots = getLongParameter("slots", 16);
	}
}

void CrankshaftF1::finish() {
	CrankshaftBase::finish();
}

CrankshaftF1::~CrankshaftF1() {
	parametersInitialised = false;
}

void CrankshaftF1::txPacket(MacPacket * msg) {
	CrankshaftBase::txPacket(msg);
	
	if ((msg->local_to % slots) == (macid() % slots)) {
		printf("Can't send message, dropping");
		txPacketFail(msg);
		++stat_tx_drop;
		tx_msg = NULL;
		return;
	}
}

void CrankshaftF1::wrapSlotCounter() {
	/* Default implementation for wrapSlotCounter. */
	if (current_slot == slots + 1)
		current_slot = 0;
}

CrankshaftBase::SlotState CrankshaftF1::getCurrentSlotState() {
	if (current_slot == (macid() % slots)) {
		/* Listening in this slot. */
		printf("listening");
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
	if (macid() == 0)
		return SSTATE_RECEIVE;
#endif

	return SSTATE_SLEEP;
}

int CrankshaftF1::slotsUntilWake(int destination) {
	int destinationSlot = destination % slots;
	
	if (destinationSlot == macid() % slots)
		return INT_MAX;
	
	destinationSlot -= current_slot;
	if (destinationSlot <= 0)
		destinationSlot += slots + 1;
	return destinationSlot;
}
