/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIRPhy.h
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

#include "UWBIRPhyLayer.h"
#include <assert.h>

Define_Module(UWBIRPhyLayer)
;

void UWBIRPhyLayer::initialize(int stage) {
	BasePhyLayer::initialize(stage);
	if (stage == 0) {

		delete radio;

		_radio = new UWBIRRadio();
		radio = _radio;

		// Radio timers
		// From Sleep mode
		_radio->setSwitchTime(UWBIRRadio::SLEEP, UWBIRRadio::RX, par(
				"timeSleepToRX"));
		_radio->setSwitchTime(UWBIRRadio::SLEEP, UWBIRRadio::TX, par(
				"timeSleepToTX"));
		_radio->setSwitchTime(UWBIRRadio::SLEEP, UWBIRRadio::SLEEP, 0);

		// From TX mode
		_radio->setSwitchTime(UWBIRRadio::TX, UWBIRRadio::SYNC, par(
				"timeTXToRX"));
		_radio->setSwitchTime(UWBIRRadio::TX, UWBIRRadio::RX, par("timeTXToRX"));

		// From RX mode
		_radio->setSwitchTime(UWBIRRadio::RX, UWBIRRadio::TX, par("timeRXToTX"));
		_radio->setSwitchTime(UWBIRRadio::RX, UWBIRRadio::SYNC, 0.000000001);
		_radio->setSwitchTime(UWBIRRadio::SYNC, UWBIRRadio::TX, par(
				"timeRXToTX"));

		// From SYNC mode
		_radio->setSwitchTime(UWBIRRadio::SYNC, UWBIRRadio::RX, 0.000000001);

		_radio->setPowerConsumption(UWBIRRadio::SLEEP, par("PSleep"));
		_radio->setPowerConsumption(UWBIRRadio::SYNC, par("PSync"));
		_radio->setPowerConsumption(UWBIRRadio::RX, par("PRx"));
		_radio->setPowerConsumption(UWBIRRadio::TX, par("PTx"));
		_radio->setPowerConsumption(UWBIRRadio::SWITCHING, par("PSwitch"));
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

	uwbpathloss = new UWBIRStochasticPathlossModel(PL0, mu_gamma, sigma_gamma,
			mu_sigma, sigma_sigma, &move, isEnabled);

	return uwbpathloss;

}

AnalogueModel* UWBIRPhyLayer::createUWBIRIEEE802154APathlossModel(
		ParameterMap & params) {
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
		error(
				"Could not find required double parameter <Threshold> in the IEEE 802.15.4A channel xml description file.");
	}
	threshold = it->second.doubleValue();
	ieee802154AChannel = new UWBIRIEEE802154APathlossModel(CM, threshold);
	return ieee802154AChannel;
}

Decider* UWBIRPhyLayer::getDeciderFromName(std::string name,
		ParameterMap& params) {

	double sensitivity;
	double syncThreshold;
	bool stats;
	bool trace;
	bool syncAlwaysSucceeds;

	ParameterMap::iterator it;

	it = params.find("sensitivity");
	if (it == params.end()) {
		error(
				"Could not find required double parameter <sensitivity> in the decider xml configuration file.");
	}
	sensitivity = it->second.doubleValue();
	// convert to real number
	sensitivity = 10*log10(sensitivity);
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

	if (name == "UWBIREDSyncOnAddress") {
		int addr;
		it = params.find("addr");
		if (it == params.end()) {
			error(
					"Could not find required int parameter <addr> in the decider xml configuration file.");
		}
		addr = it->second.longValue();
		return new UWBIREDSyncOnAddress(this, this, sensitivity, syncThreshold,
				syncAlwaysSucceeds, stats, trace, addr);
	}

	if (name == "UWBIREDSync") {
		double tmin;
		it = params.find("syncThreshold");
		if (it == params.end()) {
			error(
					"Could not find required double parameter <syncTrheshold> in the decider xml configuration file.");
		}
		tmin = it->second.doubleValue();
		return new UWBIREDSync(this, this, sensitivity, syncThreshold,
				syncAlwaysSucceeds, stats, trace, tmin);
	}

	if (name=="UWBIREnergyDetectionDeciderV2") {
	return new UWBIREnergyDetectionDeciderV2(this, this, sensitivity,
			syncThreshold, syncAlwaysSucceeds, stats, trace);
	}

	return 0;
}

void UWBIRPhyLayer::receiveBBItem(int category, const BBItem *details,
		int scopeModuleId) {
	Enter_Method_Silent();
	ChannelAccess::receiveBBItem(category, details, scopeModuleId);
	if (category == catMove) {
		EV<< "Received move information in uwbphylayer." << endl;
		if(ieee802154AChannel) {
			ieee802154AChannel->setMove(move);
		}

	}
}

void UWBIRPhyLayer::finish() {
	UWBIREnergyDetectionDeciderV2 * dec =
			static_cast<UWBIREnergyDetectionDeciderV2*> (decider);
	recordScalar("nbRandomBits", dec->getNbRandomBits());
	recordScalar("avgThreshold", dec->getAvgThreshold());
	recordScalar("nbSuccessfulSyncs", dec->getNbSuccessfulSyncs());
	recordScalar("nbFailedSyncs", dec->getNbFailedSyncs());
}
