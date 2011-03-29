#include "PhyLayerDetailed.h"
#include "PhyUtils.h"

Define_Module(PhyLayerDetailed);

Radio* PhyLayerDetailed::initializeRadio() {
    	int initialRadioState = par("initialRadioState"); //readPar("initalRadioState", (int) RadioUWBIR::SYNC);
    	double radioMinAtt = readPar("radioMinAtt", 1.0);
    	double radioMaxAtt = readPar("radioMaxAtt", 0.0);
    	int nbRadioChannels = readPar("nbRadioChannels", 1);
    	int initialRadioChannel = readPar("initialRadioChannel", 0);
    	radioDetailed = RadioDetailed::createNewRadioDetailed(initialRadioState, recordStats, radioMinAtt, radioMaxAtt,
    			initialRadioChannel, nbRadioChannels);

    	//	- switch times to TX
    	simtime_t rxToTX = readPar("timeRXToTX", 0.0);
    	simtime_t sleepToTX = readPar("timeSleepToTX", 0.0);

    	// Radio timers
    	// From Sleep mode
    	radioDetailed->setSwitchTime(RadioDetailed::SLEEP, RadioDetailed::ON, par("timeSleepToON"));
    	radioDetailed->setSwitchTime(RadioDetailed::SLEEP, RadioDetailed::SLEEP, 0);

    	// From ON mode
    	radioDetailed->setSwitchTime(RadioDetailed::ON, RadioDetailed::ON, 0);
    	radioDetailed->setSwitchTime(RadioDetailed::ON, RadioDetailed::SLEEP, 0);
    	radioDetailed->setSwitchTime(RadioDetailed::ON, RadioDetailed::TX, par("timeONToTX"));
    	radioDetailed->setSwitchTime(RadioDetailed::ON, RadioDetailed::RX, par("timeONToRX"));

    	// From TX mode
    	radioDetailed->setSwitchTime(RadioDetailed::TX, RadioDetailed::TX, 0);
    	radioDetailed->setSwitchTime(RadioDetailed::TX, RadioDetailed::ON, 0);
    	radioDetailed->setSwitchTime(RadioDetailed::TX, RadioDetailed::SLEEP, 0);
    	radioDetailed->setSwitchTime(RadioDetailed::TX, RadioDetailed::RX, par("timeTXToRX"));

    	// From RX mode
    	radioDetailed->setSwitchTime(RadioDetailed::RX, RadioDetailed::RX, 0);
    	radioDetailed->setSwitchTime(RadioDetailed::RX, RadioDetailed::ON, 0);
    	radioDetailed->setSwitchTime(RadioDetailed::RX, RadioDetailed::SLEEP, 0);
    	radioDetailed->setSwitchTime(RadioDetailed::RX, RadioDetailed::TX, par("timeRXToTX"));

    	// Radio currents:
    	setupOnCurrent = getParentModule()->par( "onCurrent" );
    	onCurrent = getParentModule()->par( "onCurrent" );

    	return radioDetailed;
}

void PhyLayerDetailed::setSwitchingCurrent(int from, int to) {
	int act = SWITCHING_ACCT;
	double current = 0;

	switch(from) {
	case Radio::RX:
		switch(to) {
		case Radio::SLEEP:
		case RadioDetailed::ON:
			current = rxCurrent;
			break;
		case Radio::TX:
			current = rxTxCurrent;
			break;
		default:
			opp_error("Unknown radio switch! From RX to %d", to);
		}
		break;

	case Radio::TX:
		switch(to) {
		case Radio::SLEEP:
		case RadioDetailed::ON:
			current = txCurrent;
			break;
		case Radio::RX:
			current = txRxCurrent;
			break;
		default:
			opp_error("Unknown radio switch! From TX to %d", to);
		}
		break;

	case Radio::SLEEP:
		switch(to) {
		case RadioDetailed::ON:
			current = onCurrent;
			break;
		case Radio::TX:
			current = setupTxCurrent;
			break;
		case Radio::RX:
			current = setupRxCurrent;
			break;
		default:
			opp_error("Unknown radio switch! From SLEEP to %d", to);
		}
		break;
	case RadioDetailed::ON:
		switch(to) {
		case Radio::TX:
			current = setupTxCurrent;
			break;
		case Radio::RX:
			current = setupRxCurrent;
			break;
		case Radio::SLEEP:
			current = onCurrent;
			break;
		default:
			opp_error("Unknown radio switch! From ON to %d", to);
		}
		break;
	default:
		opp_error("Unknown radio state: %d", from);
	}

	BatteryAccess::drawCurrent(current, act);
}
