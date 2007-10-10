/* G-MAC, fixed-sequence, using receive slots for sending to neighbours with
   same slot assignment.
*/

#include "gmacf2.h"

Define_Module_Like( GMacF2, EyesMacLayer );

int GMacF2::slots, GMacF2::bcast_slots;
bool GMacF2::parametersInitialised = false, GMacF2::slotted_bcast;

void GMacF2::initialize() {
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

void GMacF2::finish() {
	GMac::finish();
}

GMacF2::~GMacF2() {
	parametersInitialised = false;
}

void GMacF2::wrapSlotCounter() {
	/* Default implementation for wrapSlotCounter. */
	if (current_slot == slots + bcast_slots)
		current_slot = 0;
}

GMac::SlotState GMacF2::getCurrentSlotState() {
	if (current_slot == (macid() % slots)) {
		/* Listening in this slot. */
		if (tx_msg && (tx_msg->local_to % slots) == current_slot)
			printf(PRINT_MAC, "contending/listening");
		else
			printf(PRINT_MAC, "listening");
		return tx_msg && (tx_msg->local_to % slots) == current_slot ? SSTATE_SEND_RECEIVE : SSTATE_RECEIVE;
	} 

	if (tx_msg) {
		if (tx_msg->local_to == BROADCAST) {
			if (current_slot >= slots)
				printf(PRINT_MAC, "Broadcast to send. Will send = %d (%u)", macid() % bcast_slots == current_slot - slots, macid());
			
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

	/* Broadcast */
	if (current_slot >= slots)
		return SSTATE_RECEIVE;

#ifdef SINK_ALWAYS_ON // Assumes that node 0 is sink!
	if (macid() == 0)
		return SSTATE_RECEIVE;
#endif
	
	return SSTATE_SLEEP;
}

int GMacF2::slotsUntilWake(int destination) {
	int destinationSlot = destination % slots;
	
	destinationSlot -= current_slot;
	if (destinationSlot <= 0)
		destinationSlot += slots + bcast_slots;
	return destinationSlot;
}
