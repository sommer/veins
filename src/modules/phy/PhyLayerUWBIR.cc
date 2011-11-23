#include "PhyLayerUWBIR.h"

#include <cassert>

#include "DeciderUWBIREDSyncOnAddress.h"
#include "DeciderUWBIREDSync.h"
#include "MacToUWBIRPhyControlInfo.h"
#include "AirFrameUWBIR_m.h"
#include "BaseWorldUtility.h"

Define_Module(PhyLayerUWBIR);

//t_dynamic_expression_value (PhyLayerUWBIR::*ghassemzadehNLOSFPtr) (cComponent *context, t_dynamic_expression_value argv[], int argc) = &ghassemzadehNLOSFunc;
PhyLayerUWBIR::fptr ghassemzadehNLOSFPtr = &PhyLayerUWBIR::ghassemzadehNLOSFunc;
Define_NED_Function(ghassemzadehNLOSFPtr, "xml ghassemzadehNLOS()");

void PhyLayerUWBIR::initialize(int stage) {
	BasePhyLayer::initialize(stage);
	if (stage == 0) {
		numActivities = hasPar("numActivities") ? par("numActivities").longValue() : 6;

		/* parameters belong to the NIC, not just phy layer
		 *
		 * if/when variable transmit power is supported, txCurrent
		 * should be specified as an xml table of available transmit
		 * power levels and corresponding txCurrent */
		sleepCurrent = rxCurrent = decodingCurrentDelta = txCurrent = 0;
		setupRxCurrent = setupTxCurrent = rxTxCurrent = txRxCurrent = 0;
		sleepCurrent = getParentModule()->par( "sleepCurrent" );
		rxCurrent = getParentModule()->par( "rxCurrent" );
//		decodingCurrentDelta = getParentModule()->par( "decodingCurrentDelta" );
		txCurrent = getParentModule()->par( "txCurrent" );
		setupRxCurrent = getParentModule()->par( "setupRxCurrent" );
		setupTxCurrent = getParentModule()->par( "setupTxCurrent" );
		rxTxCurrent = getParentModule()->par( "rxTxCurrent" );
		txRxCurrent = getParentModule()->par( "txRxCurrent" );
		syncCurrent = getParentModule()->par( "syncCurrent" ); // assume instantaneous transitions between rx and sync
	} else if (stage == 1) {
		registerWithBattery("physical layer", numActivities);
		setRadioCurrent(uwbradio->getCurrentState());
	}

}


Radio* PhyLayerUWBIR::initializeRadio() {
	int initialRadioState = par("initialRadioState"); //readPar("initalRadioState", (int) RadioUWBIR::SYNC);
	double radioMinAtt = readPar("radioMinAtt", 1.0);
	double radioMaxAtt = readPar("radioMaxAtt", 0.0);

	uwbradio = RadioUWBIR::createNewUWBIRRadio(initialRadioState, recordStats, radioMinAtt, radioMaxAtt);

	//	- switch times to TX
	//simtime_t rxToTX = readPar("timeRXToTX", 0.0);
	//simtime_t sleepToTX = readPar("timeSleepToTX", 0.0);

	// Radio timers
	// From Sleep mode
	uwbradio->setSwitchTime(RadioUWBIR::SLEEP, RadioUWBIR::RX, par("timeSleepToRX"));
	uwbradio->setSwitchTime(RadioUWBIR::SLEEP, RadioUWBIR::TX, par("timeSleepToTX"));
	uwbradio->setSwitchTime(RadioUWBIR::SLEEP, RadioUWBIR::SLEEP, 0);

	// From TX mode
	uwbradio->setSwitchTime(RadioUWBIR::TX, RadioUWBIR::SYNC, par("timeTXToRX"));
	uwbradio->setSwitchTime(RadioUWBIR::TX, RadioUWBIR::RX, par("timeTXToRX"));

	// From RX mode
	uwbradio->setSwitchTime(RadioUWBIR::RX, RadioUWBIR::TX, par("timeRXToTX"));
	uwbradio->setSwitchTime(RadioUWBIR::RX, RadioUWBIR::SYNC, 0.000000001);
	uwbradio->setSwitchTime(RadioUWBIR::SYNC, RadioUWBIR::TX, par("timeRXToTX"));

	// From SYNC mode
	uwbradio->setSwitchTime(RadioUWBIR::SYNC, RadioUWBIR::RX, 0.000000001);

	return uwbradio;
}

AnalogueModel* PhyLayerUWBIR::getAnalogueModelFromName(std::string name,
		ParameterMap& params) {
	if (name == "UWBIRStochasticPathlossModel")
		return createUWBIRStochasticPathlossModel(params);

	if (name == "UWBIRIEEE802154APathlossModel")
		return createUWBIRIEEE802154APathlossModel(params);

	if (name == "RadioStateAnalogueModel")
		return uwbradio->getAnalogueModel();

	return 0;
}

/*
 *Returns an AnalogueModel object which will be stored in the
 * analogueModels list. All AnalogueModel objects are successively applied
 * to any signal at the beginning of its reception, and their outputs
 * (attenuations objects) are then associated to the incoming signal object.
 **/
AnalogueModel* PhyLayerUWBIR::createUWBIRStochasticPathlossModel(
		ParameterMap & params) {
	//get the pathloss exponent parameter from the config
	ParameterMap::iterator it = params.find("PL0");
	double PL0 = it->second.doubleValue();
	it = params.find("mu_gamma");
	if (it == params.end()) {
		error(
				"Could not find required double parameter <mu_gamma> in the Ghassemzadeh channel xml description file.");
	}
	double mu_gamma = it->second.doubleValue();
	it = params.find("sigma_gamma");
	if (it == params.end()) {
		error(
				"Could not find required double parameter <sigma_gamma> in the Ghassemzadeh channel xml description file.");
	}
	double sigma_gamma = it->second.doubleValue();
	it = params.find("mu_sigma");
	if (it == params.end()) {
		error(
				"Could not find required double parameter <mu_sigma> in the Ghassemzadeh channel xml description file.");
	}
	double mu_sigma = it->second.doubleValue();
	it = params.find("sigma_sigma");
	if (it == params.end()) {
		error(
				"Could not find required double parameter <sigma_sigma> in the Ghassemzadeh channel xml description file.");
	}
	double sigma_sigma = it->second.doubleValue();

	it = params.find("isEnabled");
	if (it == params.end()) {
		error(
				"Could not find required bool parameter <isEnabled> in the Ghassemzadeh channel xml description file.");
	}
	bool isEnabled = it->second.boolValue();

	bool shadowing = true;
	it = params.find("shadowing");
	if (it != params.end()) {
		shadowing = it->second.boolValue();
	}

	uwbpathloss = new UWBIRStochasticPathlossModel(PL0, mu_gamma, sigma_gamma,
			mu_sigma, sigma_sigma, isEnabled, shadowing);

	return uwbpathloss;

}

AnalogueModel* PhyLayerUWBIR::createUWBIRIEEE802154APathlossModel(ParameterMap & params) {
	int CM;
	ParameterMap::iterator it = params.find("CM");
	if (it == params.end()) {
		error(
				"Could not find required integer parameter <CM> (Channel Model) in the IEEE 802.15.4A channel xml description file.");
	}
	CM = int(it->second.longValue());

	double threshold;
	it = params.find("Threshold");
	if (it == params.end()) {
		error("Could not find required double parameter <Threshold> in the IEEE 802.15.4A channel xml description file.");
	}
	threshold = it->second.doubleValue();

	bool shadowing = false;
	it = params.find("shadowing");
	if (it != params.end()) {
		shadowing = it->second.boolValue();
	}

	ieee802154AChannel = new UWBIRIEEE802154APathlossModel(CM, threshold, shadowing);
	return ieee802154AChannel;
}

Decider* PhyLayerUWBIR::getDeciderFromName(std::string name, ParameterMap& params) {
	double syncThreshold;
	bool stats;
	bool trace;
	bool syncAlwaysSucceeds;
	ParameterMap::iterator it;

	protocolId = IEEE_802154_UWB;

	it = params.find("syncThreshold");
	if (it == params.end()) {
		error(
				"Could not find required double parameter <syncThreshold> in the decider xml configuration file.");
	}
	syncThreshold = it->second.doubleValue();

	it = params.find("stats");
	if (it == params.end()) {
		error(
				"Could not find required boolean parameter <stats> in the decider xml configuration file.");
	}
	stats = it->second.boolValue();

	it = params.find("trace");
	if (it == params.end()) {
		error(
				"Could not find required boolean parameter <trace> in the decider xml configuration file.");
	}
	trace = it->second.boolValue();

	it = params.find("syncAlwaysSucceeds");
	if (it == params.end()) {
		error(
				"Could not find required boolean parameter <syncAlwaysSucceeds> in the decider xml configuration file.");
	}
	syncAlwaysSucceeds = it->second.boolValue();

	bool alwaysFailOnDataInterference;
	it = params.find("alwaysFailOnDataInterference");
	if (it == params.end()) {
		alwaysFailOnDataInterference = false;
	} else {
	  alwaysFailOnDataInterference = it->second.boolValue();
	}

	if (name == "DeciderUWBIREDSyncOnAddress") {
		LAddress::L2Type addr;
		it = params.find("addr");
		if (it == params.end()) {
			error(
					"Could not find required int parameter <addr> in the decider xml configuration file.");
		}
		addr = LAddress::L2Type(it->second.longValue());
		uwbdecider = new DeciderUWBIREDSyncOnAddress(this, this, syncThreshold,
				syncAlwaysSucceeds, stats, trace, addr, alwaysFailOnDataInterference);
	}

	if (name == "DeciderUWBIREDSync") {
		double tmin;
		it = params.find("syncMinDuration");
		if (it == params.end()) {
			error(
					"Could not find required double parameter <syncMinDuration> in the decider xml configuration file.");
		}
		tmin = it->second.doubleValue();
		uwbdecider = new DeciderUWBIREDSync(this, this, syncThreshold, syncAlwaysSucceeds, stats, trace, tmin, alwaysFailOnDataInterference);
	}

	if (name=="DeciderUWBIRED") {
	    uwbdecider = new DeciderUWBIRED(this, this, syncThreshold, syncAlwaysSucceeds, stats, trace, alwaysFailOnDataInterference);
	}
	return uwbdecider;
}

void PhyLayerUWBIR::handleHostState(const HostState& state) {
	if (state.get() != HostState::ACTIVE && radio->getCurrentState() != Radio::SLEEP) {
		coreEV<< "host is no longer in active state (maybe FAILED, SLEEP, OFF or BROKEN), force into sleep state!" << endl;
		setRadioState(Radio::SLEEP);
		// it would be good to create a radioState OFF, as well
	}
}

void PhyLayerUWBIR::finishRadioSwitching() {
	BasePhyLayer::finishRadioSwitching();

	setRadioCurrent(radio->getCurrentState());
}

void PhyLayerUWBIR::setSwitchingCurrent(int from, int to) {
	int act = SWITCHING_ACCT;
	double current = 0;

	switch(from) {
	case RadioUWBIR::SYNC:
	case Radio::RX:
		switch(to) {
		case Radio::SLEEP:
			current = rxCurrent;
			break;
		case Radio::TX:
			current = rxTxCurrent;
			break;
			// ! transitions between rx and sync should be immediate
		default:
			opp_error("Unknown radio switch! From RX to %d", to);
			break;
		}
		break;

	case Radio::TX:
		switch(to) {
		case Radio::SLEEP:
			current = txCurrent;
			break;
		case Radio::RX:
			current = txRxCurrent;
			break;
		default:
			opp_error("Unknown radio switch! From TX to %d", to);
			break;
		}
		break;

	case Radio::SLEEP:
		switch(to) {
		case Radio::TX:
			current = setupTxCurrent;
			break;
		case Radio::RX:
			current = setupRxCurrent;
			break;
		default:
			opp_error("Unknown radio switch! From SLEEP to %d", to);
			break;
		}
		break;

	default:
		opp_error("Unknown radio state: %d", from);
		break;
	}

	BatteryAccess::drawCurrent(current, act);
}

void PhyLayerUWBIR::setRadioCurrent(int rs) {
	switch(rs) {
	case RadioUWBIR::RX:
		BatteryAccess::drawCurrent(rxCurrent, RX_ACCT);
		break;
	case RadioUWBIR::TX:
		BatteryAccess::drawCurrent(txCurrent, TX_ACCT);
		break;
	case RadioUWBIR::SLEEP:
		BatteryAccess::drawCurrent(sleepCurrent, SLEEP_ACCT);
		break;
	case RadioUWBIR::SYNC:
		BatteryAccess::drawCurrent(syncCurrent, SYNC_ACCT);
		break;
	default:
		break;
	}
}

/*
simtime_t PhyLayerUWBIR::setRadioState(int rs) {
	if(_radio->getCurrentState()==RadioUWBIR::RX && rs != RadioUWBIR::RX && rs!= RadioUWBIR::SYNC) {
		uwbdecider->cancelReception();
	}
  BasePhyLayer::setRadioState(rs);
}
*/
simtime_t PhyLayerUWBIR::setRadioState(int rs) {
	int prevState = radio->getCurrentState();

	if(rs==Radio::RX) {
		coreEV << "this is my breakpoint" << endl;
	}
	if(radio->getCurrentState()==RadioUWBIR::RX && rs != RadioUWBIR::RX && rs!= RadioUWBIR::SYNC) {
		uwbdecider->cancelReception();
	}

	simtime_t endSwitch = BasePhyLayer::setRadioState(rs);

	if(endSwitch >= 0) {
		if(radio->getCurrentState() == Radio::SWITCHING) {
						setSwitchingCurrent(prevState, rs);
		} else {
			setRadioCurrent(radio->getCurrentState());
		}
	}

	return endSwitch;
}

AirFrame *PhyLayerUWBIR::encapsMsg(cPacket *macPkt)
{
	// the cMessage passed must be a MacPacket... but no cast needed here
	// MacPkt* pkt = static_cast<MacPkt*>(msg);

	// ...and must always have a ControlInfo attached (contains Signal)
	cObject* ctrlInfo = macPkt->removeControlInfo();
	assert(ctrlInfo);

	// create the new AirFrame
	AirFrameUWBIR* frame = new AirFrameUWBIR("airframe", AIR_FRAME);

	// Retrieve the pointer to the Signal-instance from the ControlInfo-instance.
	// We are now the new owner of this instance.
	Signal* s = MacToUWBIRPhyControlInfo::getSignalFromControlInfo(ctrlInfo);
	// make sure we really obtained a pointer to an instance
	assert(s);

	// set the members
	assert(s->getDuration() > 0);
	frame->setDuration(s->getDuration());
	// copy the signal into the AirFrame
	frame->setSignal(*s);
	//set priority of AirFrames above the normal priority to ensure
	//channel consistency (before any thing else happens at a time
	//point t make sure that the channel has removed every AirFrame
	//ended at t and added every AirFrame started at t)
	frame->setSchedulingPriority(airFramePriority);
	frame->setProtocolId(myProtocolId());
	frame->setBitLength(headerLength);
	frame->setId(world->getUniqueAirFrameId());
	frame->setChannel(radio->getCurrentChannel());
	frame->setCfg(MacToUWBIRPhyControlInfo::getConfigFromControlInfo(ctrlInfo));

	// pointer and Signal not needed anymore
	delete s;
	s = 0;

	// delete the Control info
	delete ctrlInfo;
	ctrlInfo = 0;

	frame->encapsulate(macPkt);

	// --- from here on, the AirFrame is the owner of the MacPacket ---
	macPkt = 0;
	coreEV <<"AirFrame encapsulated, length: " << frame->getBitLength() << "\n";

	return frame;
}


void PhyLayerUWBIR::finish() {
	BasePhyLayer::finish();
}

