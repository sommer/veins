/*
 * PhyLayer.cc
 *
 *  Created on: 11.02.2009
 *      Author: karl
 */

#include "PhyLayer.h"
#include <Decider80211.h>

Define_Module(PhyLayer);

AnalogueModel* PhyLayer::getAnalogueModelFromName(std::string name, ParameterMap& params) {
	return BasePhyLayer::getAnalogueModelFromName(name, params);
}

Decider* PhyLayer::getDeciderFromName(std::string name, ParameterMap& params) {
	if(name == "Decider80211") {
		return initializeDecider80211(params);
	}

	return BasePhyLayer::getDeciderFromName(name, params);
}

Decider* PhyLayer::initializeDecider80211(ParameterMap& params) {
	double threshold = params["threshold"];
	return new Decider80211(this, threshold, sensitivity, findHost()->getIndex(), coreDebug);
}
