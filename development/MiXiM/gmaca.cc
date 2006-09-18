/* G-MAC, SCP-MAC type. */

#include "gmaca.h"

Define_Module_Like( GMacA, MacClass );

bool GMacA::parametersInitialised = false;

void GMacA::initialize() {
	/* Max header contains from, type, to, and clock data */
	max_header_length = ADDRESS_BYTES + TYPE_BYTES + ADDRESS_BYTES + CLOCK_BYTES;
	/* Min header contains from, type, and clock data */
	min_header_length = ADDRESS_BYTES + TYPE_BYTES + CLOCK_BYTES;
	
	GMac::initialize();

	if (!parametersInitialised) {
		parametersInitialised = true;
	}
}

void GMacA::finish() {
	GMac::finish();
}

GMacA::~GMacA() {
	parametersInitialised = false;
}

void GMacA::wrapSlotCounter() {
}

GMac::SlotState GMacA::getCurrentSlotState() {
	return tx_msg ? SSTATE_SEND_RECEIVE : SSTATE_RECEIVE;
}

int GMacA::slotsUntilWake(int destination) {
	return 0;
}
