//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

/*
 * Based on PhyLayer.cc from Karl Wessel
 * and modifications by Christopher Saloman
 */

#include "PhyLayer80211p.h"

#include "Decider80211p.h"
#include "SimplePathlossModel.h"
#include "BreakpointPathlossModel.h"
#include "LogNormalShadowing.h"
#include "JakesFading.h"
#include "PERModel.h"
#include "SimpleObstacleShadowing.h"
#include "TwoRayInterferenceModel.h"
#include "BaseConnectionManager.h"
#include <Consts80211p.h>

Define_Module(PhyLayer80211p);

/** This is needed to circumvent a bug in MiXiM that allows different header length interpretations for receiving and sending airframes*/
void PhyLayer80211p::initialize(int stage) {
	BasePhyLayer::initialize(stage);
	if (stage == 0) {
		if (par("headerLength").longValue() != PHY_HDR_TOTAL_LENGTH) {
		opp_error("The header length of the 802.11p standard is 46bit, please change your omnetpp.ini accordingly by either setting it to 46bit or removing the entry");
		}
	}
}

AnalogueModel* PhyLayer80211p::getAnalogueModelFromName(std::string name, ParameterMap& params) {

	if (name == "SimplePathlossModel") {
		return initializeSimplePathlossModel(params);
	}
	else if (name == "LogNormalShadowing")
	{
		return initializeLogNormalShadowing(params);
	}
	else if (name == "JakesFading")
	{
		return initializeJakesFading(params);
	}
	else if(name == "BreakpointPathlossModel")
	{
		return initializeBreakpointPathlossModel(params);
	} else if(name == "PERModel")
	{
		return initializePERModel(params);
	}
	else if (name == "SimpleObstacleShadowing")
	{
		return initializeSimpleObstacleShadowing(params);
	}
	else if (name == "TwoRayInterferenceModel")
	{
		return initializeTwoRayInterferenceModel(params);
	}

	return BasePhyLayer::getAnalogueModelFromName(name, params);
}

AnalogueModel* PhyLayer80211p::initializeLogNormalShadowing(ParameterMap& params) {
	double mean = params["mean"].doubleValue();
	double stdDev = params["stdDev"].doubleValue();
	simtime_t interval = params["interval"].doubleValue();

	return new LogNormalShadowing(mean, stdDev, interval);
}

AnalogueModel* PhyLayer80211p::initializeJakesFading(ParameterMap& params) {
	int fadingPaths = params["fadingPaths"].longValue();
	simtime_t delayRMS = params["delayRMS"].doubleValue();
	simtime_t interval = params["interval"].doubleValue();

	double carrierFrequency = 5.890e+9;
	if (params.count("carrierFrequency") > 0) {
		carrierFrequency = params["carrierFrequency"];
	}
	else {
		if (cc->hasPar("carrierFrequency")) {
			carrierFrequency = cc->par("carrierFrequency").doubleValue();
		}
	}

	return new JakesFading(fadingPaths, delayRMS, carrierFrequency, interval);
}

AnalogueModel* PhyLayer80211p::initializeBreakpointPathlossModel(ParameterMap& params) {
	double alpha1 =-1, alpha2=-1, breakpointDistance=-1;
	double L01=-1, L02=-1;
	double carrierFrequency = 5.890e+9;
	bool useTorus = world->useTorus();
	const Coord& playgroundSize = *(world->getPgs());
	ParameterMap::iterator it;

	it = params.find("alpha1");
	if ( it != params.end() ) // parameter alpha1 has been specified in config.xml
	{
		// set alpha1
		alpha1 = it->second.doubleValue();
		coreEV << "createPathLossModel(): alpha1 set from config.xml to " << alpha1 << endl;
		// check whether alpha is not smaller than specified in ConnectionManager
		if(cc->hasPar("alpha") && alpha1 < cc->par("alpha").doubleValue())
		{
	        // throw error
			opp_error("TestPhyLayer::createPathLossModel(): alpha can't be smaller than specified in \
	               ConnectionManager. Please adjust your config.xml file accordingly");
		}
	}
	it = params.find("L01");
	if(it != params.end()) {
		L01 = it->second.doubleValue();
	}
	it = params.find("L02");
	if(it != params.end()) {
		L02 = it->second.doubleValue();
	}

	it = params.find("alpha2");
	if ( it != params.end() ) // parameter alpha1 has been specified in config.xml
	{
		// set alpha2
		alpha2 = it->second.doubleValue();
		coreEV << "createPathLossModel(): alpha2 set from config.xml to " << alpha2 << endl;
		// check whether alpha is not smaller than specified in ConnectionManager
		if(cc->hasPar("alpha") && alpha2 < cc->par("alpha").doubleValue())
		{
	        // throw error
			opp_error("TestPhyLayer::createPathLossModel(): alpha can't be smaller than specified in \
	               ConnectionManager. Please adjust your config.xml file accordingly");
		}
	}
	it = params.find("breakpointDistance");
	if ( it != params.end() ) // parameter alpha1 has been specified in config.xml
	{
		breakpointDistance = it->second.doubleValue();
		coreEV << "createPathLossModel(): breakpointDistance set from config.xml to " << alpha2 << endl;
		// check whether alpha is not smaller than specified in ConnectionManager
	}

	// get carrierFrequency from config
	it = params.find("carrierFrequency");

	if ( it != params.end() ) // parameter carrierFrequency has been specified in config.xml
	{
		// set carrierFrequency
		carrierFrequency = it->second.doubleValue();
		coreEV << "createPathLossModel(): carrierFrequency set from config.xml to " << carrierFrequency << endl;

		// check whether carrierFrequency is not smaller than specified in ConnectionManager
		if(cc->hasPar("carrierFrequency") && carrierFrequency < cc->par("carrierFrequency").doubleValue())
		{
			// throw error
			opp_error("TestPhyLayer::createPathLossModel(): carrierFrequency can't be smaller than specified in \
	               ConnectionManager. Please adjust your config.xml file accordingly");
		}
	}
	else // carrierFrequency has not been specified in config.xml
	{
		if (cc->hasPar("carrierFrequency")) // parameter carrierFrequency has been specified in ConnectionManager
		{
			// set carrierFrequency according to ConnectionManager
			carrierFrequency = cc->par("carrierFrequency").doubleValue();
			coreEV << "createPathLossModel(): carrierFrequency set from ConnectionManager to " << carrierFrequency << endl;
		}
		else // carrierFrequency has not been specified in ConnectionManager
		{
			// keep carrierFrequency at default value
			coreEV << "createPathLossModel(): carrierFrequency set from default value to " << carrierFrequency << endl;
		}
	}

	if(alpha1 ==-1 || alpha2==-1 || breakpointDistance==-1 || L01==-1 || L02==-1) {
		opp_error("Undefined parameters for breakpointPathlossModel. Please check your configuration.");
	}

	return new BreakpointPathlossModel(L01, L02, alpha1, alpha2, breakpointDistance, carrierFrequency, useTorus, playgroundSize, coreDebug);

}

AnalogueModel* PhyLayer80211p::initializeTwoRayInterferenceModel(ParameterMap& params) {
	double dielectricConstant= params["DielectricConstant"].doubleValue();

	return new TwoRayInterferenceModel(dielectricConstant, coreDebug);
}

AnalogueModel* PhyLayer80211p::initializeSimplePathlossModel(ParameterMap& params){

	// init with default value
	double alpha = 2.0;
	double carrierFrequency = 5.890e+9;
	bool useTorus = world->useTorus();
	const Coord& playgroundSize = *(world->getPgs());

	// get alpha-coefficient from config
	ParameterMap::iterator it = params.find("alpha");

	if ( it != params.end() ) // parameter alpha has been specified in config.xml
	{
		// set alpha
		alpha = it->second.doubleValue();
		coreEV << "createPathLossModel(): alpha set from config.xml to " << alpha << endl;

		// check whether alpha is not smaller than specified in ConnectionManager
		if(cc->hasPar("alpha") && alpha < cc->par("alpha").doubleValue())
		{
	        // throw error
			opp_error("TestPhyLayer::createPathLossModel(): alpha can't be smaller than specified in \
	               ConnectionManager. Please adjust your config.xml file accordingly");
		}
	}
	else // alpha has not been specified in config.xml
	{
		if (cc->hasPar("alpha")) // parameter alpha has been specified in ConnectionManager
		{
			// set alpha according to ConnectionManager
			alpha = cc->par("alpha").doubleValue();
			coreEV << "createPathLossModel(): alpha set from ConnectionManager to " << alpha << endl;
		}
		else // alpha has not been specified in ConnectionManager
		{
			// keep alpha at default value
			coreEV << "createPathLossModel(): alpha set from default value to " << alpha << endl;
		}
	}

	// get carrierFrequency from config
	it = params.find("carrierFrequency");

	if ( it != params.end() ) // parameter carrierFrequency has been specified in config.xml
	{
		// set carrierFrequency
		carrierFrequency = it->second.doubleValue();
		coreEV << "createPathLossModel(): carrierFrequency set from config.xml to " << carrierFrequency << endl;

		// check whether carrierFrequency is not smaller than specified in ConnectionManager
		if(cc->hasPar("carrierFrequency") && carrierFrequency < cc->par("carrierFrequency").doubleValue())
		{
			// throw error
			opp_error("TestPhyLayer::createPathLossModel(): carrierFrequency can't be smaller than specified in \
	               ConnectionManager. Please adjust your config.xml file accordingly");
		}
	}
	else // carrierFrequency has not been specified in config.xml
	{
		if (cc->hasPar("carrierFrequency")) // parameter carrierFrequency has been specified in ConnectionManager
		{
			// set carrierFrequency according to ConnectionManager
			carrierFrequency = cc->par("carrierFrequency").doubleValue();
			coreEV << "createPathLossModel(): carrierFrequency set from ConnectionManager to " << carrierFrequency << endl;
		}
		else // carrierFrequency has not been specified in ConnectionManager
		{
			// keep carrierFrequency at default value
			coreEV << "createPathLossModel(): carrierFrequency set from default value to " << carrierFrequency << endl;
		}
	}

	return new SimplePathlossModel(alpha, carrierFrequency, useTorus, playgroundSize, coreDebug);

}

AnalogueModel* PhyLayer80211p::initializePERModel(ParameterMap& params) {
	double per = params["packetErrorRate"].doubleValue();
	return new PERModel(per);
}

Decider* PhyLayer80211p::getDeciderFromName(std::string name, ParameterMap& params) {
	if(name == "Decider80211p") {
		protocolId = IEEE_80211;
		return initializeDecider80211p(params);
	}
	return BasePhyLayer::getDeciderFromName(name, params);
}

AnalogueModel* PhyLayer80211p::initializeSimpleObstacleShadowing(ParameterMap& params){

	// init with default value
	double carrierFrequency = 5.890e+9;
	bool useTorus = world->useTorus();
	const Coord& playgroundSize = *(world->getPgs());

	ParameterMap::iterator it;

	// get carrierFrequency from config
	it = params.find("carrierFrequency");

	if ( it != params.end() ) // parameter carrierFrequency has been specified in config.xml
	{
		// set carrierFrequency
		carrierFrequency = it->second.doubleValue();
		coreEV << "initializeSimpleObstacleShadowing(): carrierFrequency set from config.xml to " << carrierFrequency << endl;

		// check whether carrierFrequency is not smaller than specified in ConnectionManager
		if(cc->hasPar("carrierFrequency") && carrierFrequency < cc->par("carrierFrequency").doubleValue())
		{
			// throw error
			opp_error("initializeSimpleObstacleShadowing(): carrierFrequency can't be smaller than specified in ConnectionManager. Please adjust your config.xml file accordingly");
		}
	}
	else // carrierFrequency has not been specified in config.xml
	{
		if (cc->hasPar("carrierFrequency")) // parameter carrierFrequency has been specified in ConnectionManager
		{
			// set carrierFrequency according to ConnectionManager
			carrierFrequency = cc->par("carrierFrequency").doubleValue();
			coreEV << "createPathLossModel(): carrierFrequency set from ConnectionManager to " << carrierFrequency << endl;
		}
		else // carrierFrequency has not been specified in ConnectionManager
		{
			// keep carrierFrequency at default value
			coreEV << "createPathLossModel(): carrierFrequency set from default value to " << carrierFrequency << endl;
		}
	}

	ObstacleControl* obstacleControlP = ObstacleControlAccess().getIfExists();
	if (!obstacleControlP) opp_error("initializeSimpleObstacleShadowing(): cannot find ObstacleControl module");
	return new SimpleObstacleShadowing(*obstacleControlP, carrierFrequency, useTorus, playgroundSize, coreDebug);
}

Decider* PhyLayer80211p::initializeDecider80211p(ParameterMap& params) {
	double centerFreq = params["centerFrequency"];
	Decider80211p* dec = new Decider80211p(this, sensitivity, centerFreq, findHost()->getIndex(), coreDebug);
	dec->setPath(getParentModule()->getFullPath());
	return dec;
}

void PhyLayer80211p::changeListeningFrequency(double freq) {
	Decider80211p* dec = dynamic_cast<Decider80211p*>(decider);
	assert(dec);
	dec->changeFrequency(freq);
}
