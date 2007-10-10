/* G-MAC, fixed-sequence, using broadcast slots for sending to neighbours with
   same slot assignment.
*/

#include "gmacf3.h"

Define_Module_Like( GMacF3, MacClass );

int GMacF3::slots, GMacF3::bcast_slots;
bool GMacF3::parametersInitialised = false, GMacF3::slotted_bcast;

void GMacF3::initialize() {
	/* Max header contains from, type, to, and clock data */
	max_header_length = ADDRESS_BYTES + TYPE_BYTES + ADDRESS_BYTES + CLOCK_BYTES;
	/* Min header contains from, type, and clock data */
	min_header_length = ADDRESS_BYTES + TYPE_BYTES + CLOCK_BYTES;
	
	GMac::initialize();

	if (!parametersInitialised) {
		parametersInitialised = true;
		slots = getLongParameter("slots", 16);
		bcast_slots = getLongParameter("bcastSlots", 1);
		slotted_bcast = getBoolParameter("slottedBcast", false);
	}
}

void GMacF3::finish() {
	GMac::finish();
}

GMacF3::~GMacF3() {
	parametersInitialised = false;
}

void GMacF3::wrapSlotCounter() {
	/* Default implementation for wrapSlotCounter. */
	if (current_slot == slots + bcast_slots)
		current_slot = 0;
}

GMac::SlotState GMacF3::getCurrentSlotState() {
	if (current_slot == (macid() % slots)) {
		/* Listening in this slot. */
		printf(PRINT_MAC, "listening");
		return SSTATE_RECEIVE;
	} 

	if (tx_msg) {
		if (tx_msg->local_to == BROADCAST) {
			if (current_slot >= slots && (!slotted_bcast || macid() % bcast_slots == current_slot - slots))
				return SSTATE_SEND_RECEIVE;
#ifdef SINK_ALWAYS_ON // Assumes that node 0 is sink!
		} else if ((tx_msg->local_to % slots) == current_slot || (tx_msg->local_to == 0 && current_slot < slots)) {
#else
		} else if ((tx_msg->local_to % slots) == current_slot) {
#endif
			/* Unicast message for a node awake in this slot. */
			return SSTATE_SEND;
		}
	}

	/* If this is a broadcast slot, then
	   if we have a message to send for a node that wakes up at the same time
	   as we do, and we don't do slotted broadcast or this is our broadcast
	   slot, then send that message. Otherwise listen for others sending. */
	if (current_slot >= slots)
		return tx_msg && (tx_msg->local_to % slots) == (macid() % slots) &&
			(!slotted_bcast || macid() % bcast_slots == current_slot - slots) ? SSTATE_SEND_RECEIVE : SSTATE_RECEIVE;

#ifdef SINK_ALWAYS_ON // Assumes that node 0 is sink!
	if (macid() == 0)
		return SSTATE_RECEIVE;
#endif

	return SSTATE_SLEEP;
}

int GMacF3::slotsUntilWake(int destination) {
	int destinationSlot = destination % slots;
	
	if (destinationSlot == macid() % slots)
		destinationSlot = slots;
	
	destinationSlot -= current_slot;
	if (destinationSlot <= 0)
		destinationSlot += slots + bcast_slots;
	return destinationSlot;
}
