#include "PhyLayerDetailed.h"
#include "PhyUtils.h"
#include "AirFrameMultiChannel_m.h"

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

void PhyLayerDetailed::setCurrentRadioChannel(int newRadioChannel) {
	radioDetailed->setCurrentChannel(newRadioChannel);
}

int PhyLayerDetailed::getCurrentRadioChannel() {
	return radioDetailed->getCurrentChannel();
}

int PhyLayerDetailed::getNbRadioChannels() {
	return par("nbRadioChannels");
}

void PhyLayerDetailed::getChannelInfo(simtime_t from, simtime_t to, AirFrameVector& out) {
	AirFrameVector tmp;
	channelInfo.getAirFrames(from, to, tmp);
	AirFrameVector::iterator it;
	for(it=tmp.begin(); it != tmp.end(); it++) {
		AirFrameMultiChannel* af = check_and_cast<AirFrameMultiChannel*>(*it);
		if(af->getChannel() == radioDetailed->getCurrentChannel()) {
			out.push_back(af);
		}
	}
}

AirFrame *PhyLayerDetailed::encapsMsg(cPacket *macPkt)
{
	// the cMessage passed must be a MacPacket... but no cast needed here
	// MacPkt* pkt = static_cast<MacPkt*>(msg);

	// ...and must always have a ControlInfo attached (contains Signal)
	cObject* ctrlInfo = macPkt->removeControlInfo();
	assert(ctrlInfo);

	MacToPhyControlInfo* macToPhyCI = static_cast<MacToPhyControlInfo*>(ctrlInfo);

	// Retrieve the pointer to the Signal-instance from the ControlInfo-instance.
	// We are now the new owner of this instance.
	Signal* s = macToPhyCI->retrieveSignal();


	// delete the Control info
	delete macToPhyCI;
	macToPhyCI = 0;
	ctrlInfo = 0;

	// make sure we really obtained a pointer to an instance
	assert(s);

	// put host move pattern to Signal
	s->setMove(move);

	// create the new AirFrame
	AirFrameMultiChannel* frame = new AirFrameMultiChannel(macPkt->getName(), AIR_FRAME);

	//set priority of AirFrames above the normal priority to ensure
	//channel consistency (before any thing else happens at a time
	//point t make sure that the channel has removed every AirFrame
	//ended at t and added every AirFrame started at t)
	frame->setSchedulingPriority(airFramePriority);

	// set the members
	assert(s->getSignalLength() > 0);
	frame->setDuration(s->getSignalLength());
	// copy the signal into the AirFrame
	frame->setSignal(*s);
	frame->setBitLength(headerLength);

	frame->setChannel(radioDetailed->getCurrentChannel());
	// pointer and Signal not needed anymore
	delete s;
	s = 0;

	frame->setId(world->getUniqueAirFrameId());
	frame->encapsulate(macPkt);

	// --- from here on, the AirFrame is the owner of the MacPacket ---
	macPkt = 0;
	EV <<"AirFrame encapsulated, length: " << frame->getBitLength() << "\n";

	return frame;
}
