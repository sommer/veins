/*
 * PhyLayer.cc
 *
 *  Created on: 11.02.2009
 *      Author: karl
 */

#include "PhyLayer.h"

#include "Decider80211.h"
#include "Decider802154Narrow.h"
#include "SimplePathlossModel.h"
#include "BreakpointPathlossModel.h"
#include "LogNormalShadowing.h"
#include "SNRThresholdDecider.h"
#include "JakesFading.h"
#include "PERModel.h"
#include "SimpleObstacleShadowing.h"
#include "TwoRayInterferenceModel.h"

#include "BaseConnectionManager.h"

Define_Module(PhyLayer);

AnalogueModel* PhyLayer::getAnalogueModelFromName(std::string name, ParameterMap& params) {

	if (name == "SimplePathlossModel")
	{
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

AnalogueModel* PhyLayer::initializeLogNormalShadowing(ParameterMap& params){
	double mean = params["mean"].doubleValue();
	double stdDev = params["stdDev"].doubleValue();
	simtime_t interval = params["interval"].doubleValue();

	return new LogNormalShadowing(mean, stdDev, interval);
}

AnalogueModel* PhyLayer::initializeJakesFading(ParameterMap& params){
	int fadingPaths = params["fadingPaths"].longValue();
	simtime_t delayRMS = params["delayRMS"].doubleValue();
	simtime_t interval = params["interval"].doubleValue();

	double carrierFrequency = 2.412e+9;
	if(params.count("carrierFrequency") > 0) {
		carrierFrequency = params["carrierFrequency"];
	}
	else {
		if (cc->hasPar("carrierFrequency")) {
			carrierFrequency = cc->par("carrierFrequency").doubleValue();
		}
	}

	return new JakesFading(fadingPaths, delayRMS, carrierFrequency, interval);
}

AnalogueModel* PhyLayer::initializeBreakpointPathlossModel(ParameterMap& params) {
	double alpha1 =-1, alpha2=-1, breakpointDistance=-1;
	double L01=-1, L02=-1;
	double carrierFrequency = 2.412e+9;
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

AnalogueModel* PhyLayer::initializeTwoRayInterferenceModel(ParameterMap& params) {
	double dielectricConstant= params["DielectricConstant"].doubleValue();

	return new TwoRayInterferenceModel(dielectricConstant, coreDebug);
}

AnalogueModel* PhyLayer::initializeSimplePathlossModel(ParameterMap& params){

	// init with default value
	double alpha = 3.5;
	double carrierFrequency = 2.412e+9;
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

AnalogueModel* PhyLayer::initializePERModel(ParameterMap& params) {
	double per = params["packetErrorRate"].doubleValue();
	return new PERModel(per);
}

Decider* PhyLayer::getDeciderFromName(std::string name, ParameterMap& params) {
	if(name == "Decider80211") {
		protocolId = IEEE_80211;
		return initializeDecider80211(params);
	}
	else if(name == "SNRThresholdDecider"){
		protocolId = GENERIC;
		return initializeSNRThresholdDecider(params);
	}
	else if(name == "Decider802154Narrow") {
		protocolId = IEEE_802154_NARROW;
		return initializeDecider802154Narrow(params);
	}

	return BasePhyLayer::getDeciderFromName(name, params);
}

AnalogueModel* PhyLayer::initializeSimpleObstacleShadowing(ParameterMap& params){

	// init with default value
	double carrierFrequency = 2.412e+9;
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

Decider* PhyLayer::initializeDecider80211(ParameterMap& params) {
	double threshold = params["threshold"];
	return new Decider80211(this, threshold, sensitivity,
							radio->getCurrentChannel(),
							findHost()->getIndex(), coreDebug);
}

Decider* PhyLayer::initializeDecider802154Narrow(ParameterMap& params) {
	int sfdLength = params["sfdLength"];
	double berLowerBound = params["berLowerBound"];
	std::string modulation = params["modulation"].stringValue();
	return new Decider802154Narrow(this, findHost()->getIndex(), coreDebug, sfdLength, berLowerBound, modulation, headerLength, recordStats);
}

Decider* PhyLayer::initializeSNRThresholdDecider(ParameterMap& params)
{
	double snrThreshold = 0;
	if(params.count("snrThreshold") == 1) {
		snrThreshold = params["snrThreshold"];
	}
	else if(params.count("threshold") == 1) {
		snrThreshold = params["threshold"];
	}
	else {
		opp_warning("No SNR threshold defined in config.xml for Decider!");
	}

	double busyThreshold = sensitivity;
	if(params.count("busyThreshold") == 0) {
		ev << "No busy threshold defined for SNRThresholdDecider. Using"
		   << " phy layers sensitivity as busy threshold." << endl;
	} else {
		busyThreshold = params["busyThreshold"];
	}

	return new SNRThresholdDecider(this, snrThreshold,
								   sensitivity, busyThreshold,
								   findHost()->getIndex(), coreDebug);
}
