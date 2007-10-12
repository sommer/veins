/* Crankshaft, fixed-sequence, using receive slots for sending to neighbours with
   same slot assignment.

   This is Crankshaft as defined in the original paper
   "G.P. Halkes and K.G. Langendoen (2007). Crankshaft: An Energy-Efficient MAC-Protocol For 
   Dense Wireless Sensor Networks. In Proceedings of the 4th European Conference on Wireless Sensor Networks (EWSN 2007)"
*/

#include "crankshaft.h"

Define_Module_Like( Crankshaft, EyesMacLayer );

int Crankshaft::slots, Crankshaft::bcast_slots;
bool Crankshaft::parametersInitialised = false, Crankshaft::slotted_bcast;

void Crankshaft::initialize() {
	/* Max header contains from, type, to, and clock data */
	max_header_length = ADDRESS_BYTES + TYPE_BYTES + ADDRESS_BYTES + CLOCK_BYTES;
	/* Min header contains from, type, and clock data */
	min_header_length = ADDRESS_BYTES + TYPE_BYTES + CLOCK_BYTES;
	
	CrankshaftBase::initialize();

	if (!parametersInitialised) {
		parametersInitialised = true;
		slots = getLongParameter("slots", 16);
		bcast_slots = getLongParameter("bcastSlots", 1);
		slotted_bcast = getBoolParameter("slottedBcast", false);
	}
}

void Crankshaft::finish() {
	CrankshaftBase::finish();
}

Crankshaft::~Crankshaft() {
	parametersInitialised = false;
}

void Crankshaft::wrapSlotCounter() {
	/* Default implementation for wrapSlotCounter. */
	if (current_slot == slots + bcast_slots)
		current_slot = 0;
}

Crankshaft::SlotState Crankshaft::getCurrentSlotState() {
	if (current_slot == (macid() % slots)) {
		/* Listening in this slot. */
		if (tx_msg && (tx_msg->local_to % slots) == current_slot)
			printf("contending/listening");
		else
			printf("listening");
		return tx_msg && (tx_msg->local_to % slots) == current_slot ? SSTATE_SEND_RECEIVE : SSTATE_RECEIVE;
	} 

	if (tx_msg) {
		if (tx_msg->local_to == BROADCAST) {
			if (current_slot >= slots)
				printf("Broadcast to send. Will send = %d (%u)", macid() % bcast_slots == current_slot - slots, macid());
			
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

int Crankshaft::slotsUntilWake(int destination) {
	int destinationSlot = destination % slots;
	
	destinationSlot -= current_slot;
	if (destinationSlot <= 0)
		destinationSlot += slots + bcast_slots;
	return destinationSlot;
}
