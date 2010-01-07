/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIRPhyLayer.h
 *
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 * 				Systems Engineering
 *              Real-Time Software and Networking
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 * description: this physical layer models an ultra wideband impulse radio channel.
 ***************************************************************************/
//
// Physical layer that models an Ultra Wideband Impulse Radio transceiver.
// See the following publication for more information.
//  A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
// for Network Simulation, Jérôme Rousselot, Jean-Dominique Decotignie,
// Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
// http://portal.acm.org/citation.cfm?id=1537714
//

#include "UWBIRPhyLayer.h"
#include <assert.h>

Define_Module(UWBIRPhyLayer);

void UWBIRPhyLayer::initialize(int stage) {
	BasePhyLayer::initialize(stage);
	if (stage == 0) {

		delete radio;

		_radio = new UWBIRRadio();
		radio = _radio;

		numActivities = 6;

		// Radio timers
		// From Sleep mode
		_radio->setSwitchTime(UWBIRRadio::SLEEP, UWBIRRadio::RX, par("timeSleepToRX"));
		_radio->setSwitchTime(UWBIRRadio::SLEEP, UWBIRRadio::TX, par("timeSleepToTX"));
		_radio->setSwitchTime(UWBIRRadio::SLEEP, UWBIRRadio::SLEEP, 0);

		// From TX mode
		_radio->setSwitchTime(UWBIRRadio::TX, UWBIRRadio::SYNC, par("timeTXToRX"));
		_radio->setSwitchTime(UWBIRRadio::TX, UWBIRRadio::RX, par("timeTXToRX"));

		// From RX mode
		_radio->setSwitchTime(UWBIRRadio::RX, UWBIRRadio::TX, par("timeRXToTX"));
		_radio->setSwitchTime(UWBIRRadio::RX, UWBIRRadio::SYNC, 0.000000001);
		_radio->setSwitchTime(UWBIRRadio::SYNC, UWBIRRadio::TX, par("timeRXToTX"));

		// From SYNC mode
		_radio->setSwitchTime(UWBIRRadio::SYNC, UWBIRRadio::RX, 0.000000001);


		// Power consumption (from PhyLayerBattery)
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
		cModule* macModule = getParentModule()->getSubmodule("mac");
		int macaddress = (check_and_cast<UWBIRMac*>(macModule))->getmacaddress();
		std::ostringstream stream;
		stream << "powerConsumption-" << macaddress;
		_radio->setName(stream.str());  // get MAC address and add it
		registerWithBattery("physical layer", numActivities);
		BatteryAccess::drawCurrent(rxCurrent, RX_ACCT);
	}

}


AnalogueModel* UWBIRPhyLayer::getAnalogueModelFromName(std::string name,
		ParameterMap& params) {
	if (name == "UWBIRStochasticPathlossModel")
		return createUWBIRStochasticPathlossModel(params);

	if (name == "UWBIRIEEE802154APathlossModel")
		return createUWBIRIEEE802154APathlossModel(params);

	if (name == "RadioStateAnalogueModel")
		return _radio->getAnalogueModel();

	return 0;
}

/*
 *Returns an AnalogueModel object which will be stored in the
 * analogueModels list. All AnalogueModel objects are successively applied
 * to any signal at the beginning of its reception, and their outputs
 * (attenuations objects) are then associated to the incoming signal object.
 **/
AnalogueModel* UWBIRPhyLayer::createUWBIRStochasticPathlossModel(
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
			mu_sigma, sigma_sigma, &move, isEnabled, shadowing);

	return uwbpathloss;

}

AnalogueModel* UWBIRPhyLayer::createUWBIRIEEE802154APathlossModel(ParameterMap & params) {
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

	ieee802154AChannel = new UWBIRIEEE802154APathlossModel(CM, threshold, &move, shadowing);
	return ieee802154AChannel;
}

Decider* UWBIRPhyLayer::getDeciderFromName(std::string name, ParameterMap& params) {
	double syncThreshold;
	bool stats;
	bool trace;
	bool syncAlwaysSucceeds;
	ParameterMap::iterator it;

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

	if (name == "UWBIREDSyncOnAddress") {
		int addr;
		it = params.find("addr");
		if (it == params.end()) {
			error(
					"Could not find required int parameter <addr> in the decider xml configuration file.");
		}
		addr = it->second.longValue();
		uwbdecider = new UWBIREDSyncOnAddress(this, this, syncThreshold,
				syncAlwaysSucceeds, stats, trace, addr, alwaysFailOnDataInterference);
	}

	if (name == "UWBIREDSync") {
		double tmin;
		it = params.find("syncMinDuration");
		if (it == params.end()) {
			error(
					"Could not find required double parameter <syncMinDuration> in the decider xml configuration file.");
		}
		tmin = it->second.doubleValue();
		uwbdecider = new UWBIREDSync(this, this, syncThreshold, syncAlwaysSucceeds, stats, trace, tmin, alwaysFailOnDataInterference);
	}

	if (name=="UWBIREnergyDetectionDeciderV2") {
	    uwbdecider = new UWBIREnergyDetectionDeciderV2(this, this, syncThreshold, syncAlwaysSucceeds, stats, trace, alwaysFailOnDataInterference);
	}
	return uwbdecider;
}

void UWBIRPhyLayer::receiveBBItem(int category, const BBItem *details,
		int scopeModuleId) {
	Enter_Method_Silent();
	ChannelAccess::receiveBBItem(category, details, scopeModuleId);
	if (category == catMove) {
		EV<< "Received move information in uwbphylayer." << endl;

	}
}

void UWBIRPhyLayer::handleAirFrame(cMessage* msg) {
	if (utility->getHostState().get() == HostState::FAILED) {
		EV<< "host has FAILED, dropping msg " << msg->getName() << endl;
		delete msg;
		return;
	}
	BasePhyLayer::handleAirFrame(msg);
}

void UWBIRPhyLayer::finishRadioSwitching() {
	BasePhyLayer::finishRadioSwitching();
	setRadioCurrent(_radio->getCurrentState());
}

void UWBIRPhyLayer::handleHostState(const HostState& state) {
	// handles only battery consumption

	HostState::States hostState = state.get();

	switch (hostState) {
	case HostState::FAILED:
		EV<< "t = " << simTime() << " host state FAILED" << endl;
		// it would be good to create a radioState OFF, as well
		break;
	default:
		break;
	}
}

void UWBIRPhyLayer::setSwitchingCurrent(int from, int to) {
	int act = SWITCHING_ACCT;
	double current = 0;

	switch(from) {
	case Radio::SYNC:
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
		}
		break;

	default:
		opp_error("Unknown radio state: %d", from);
	}

	BatteryAccess::drawCurrent(current, act);
}

void UWBIRPhyLayer::setRadioCurrent(int rs) {
	switch(rs) {
	case UWBIRRadio::RX:
		BatteryAccess::drawCurrent(rxCurrent, RX_ACCT);
		break;
	case UWBIRRadio::TX:
		BatteryAccess::drawCurrent(txCurrent, TX_ACCT);
		break;
	case UWBIRRadio::SLEEP:
		BatteryAccess::drawCurrent(sleepCurrent, SLEEP_ACCT);
		break;
	case UWBIRRadio::SYNC:
		BatteryAccess::drawCurrent(syncCurrent, SYNC_ACCT);
	default:
		break;
	}
}

/*
simtime_t UWBIRPhyLayer::setRadioState(int rs) {
	if(_radio->getCurrentState()==UWBIRRadio::RX && rs != UWBIRRadio::RX && rs!= UWBIRRadio::SYNC) {
		uwbdecider->cancelReception();
	}
  BasePhyLayer::setRadioState(rs);
}
*/
simtime_t UWBIRPhyLayer::setRadioState(int rs) {
	int prevState = radio->getCurrentState();

	if(radio->getCurrentState()==UWBIRRadio::RX && rs != UWBIRRadio::RX && rs!= UWBIRRadio::SYNC) {
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


void UWBIRPhyLayer::finish() {
	UWBIREnergyDetectionDeciderV2 * dec =
			static_cast<UWBIREnergyDetectionDeciderV2*> (decider);
	recordScalar("nbRandomBits", dec->getNbRandomBits());
	recordScalar("avgThreshold", dec->getAvgThreshold());
	recordScalar("nbSuccessfulSyncs", dec->getNbSuccessfulSyncs());
	recordScalar("nbFailedSyncs", dec->getNbFailedSyncs());
}
