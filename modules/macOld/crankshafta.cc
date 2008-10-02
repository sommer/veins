/* Crankshaft, SCP-MAC type. */

#include "crankshafta.h"

//Define_Module_Like( CrankshaftA, EyesMacLayer );

bool CrankshaftA::parametersInitialised = false;

void CrankshaftA::initialize() {
	/* Max header contains from, type, to, and clock data */
	max_header_length = ADDRESS_BYTES + TYPE_BYTES + ADDRESS_BYTES + CLOCK_BYTES;
	/* Min header contains from, type, and clock data */
	min_header_length = ADDRESS_BYTES + TYPE_BYTES + CLOCK_BYTES;
	
	CrankshaftBase::initialize();

	if (!parametersInitialised) {
		parametersInitialised = true;
	}
}

void CrankshaftA::finish() {
	CrankshaftBase::finish();
}

CrankshaftA::~CrankshaftA() {
	parametersInitialised = false;
}

void CrankshaftA::wrapSlotCounter() {
}

CrankshaftBase::SlotState CrankshaftA::getCurrentSlotState() {
	return tx_msg ? SSTATE_SEND_RECEIVE : SSTATE_RECEIVE;
}

int CrankshaftA::slotsUntilWake(int destination) {
	return 0;
}
