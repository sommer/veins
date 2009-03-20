/*
 * PhyLayer.cc
 *
 *  Created on: 11.02.2009
 *      Author: karl
 */

#include "PhyLayer.h"
#include <Decider80211.h>
#include <SimplePathlossModel.h>
#include <LogNormalShadowing.h>
#include <SNRThresholdDecider.h>
#include <JakesFading.h>

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

	double carrierFrequency = 2.412e+9;
	if(params.count("carrierFrequency") > 0) {
		carrierFrequency = params["carrierFrequency"];
	}
	else {
		if (cc->hasPar("carrierFrequency")) {
			carrierFrequency = cc->par("carrierFrequency").doubleValue();
		}
	}

	return new JakesFading(fadingPaths, delayRMS, &move, carrierFrequency);
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

	return new SimplePathlossModel(alpha, carrierFrequency, &move, useTorus, playgroundSize, coreDebug);

}

Decider* PhyLayer::getDeciderFromName(std::string name, ParameterMap& params) {
	if(name == "Decider80211") {
		return initializeDecider80211(params);
	}
	else if(name == "SNRThresholdDecider"){
		return initializeSNRThresholdDecider(params);
	}

	return BasePhyLayer::getDeciderFromName(name, params);
}

Decider* PhyLayer::initializeDecider80211(ParameterMap& params) {
	double threshold = params["threshold"];
	return new Decider80211(this, threshold, sensitivity, findHost()->getIndex(), coreDebug);
}

Decider* PhyLayer::initializeSNRThresholdDecider(ParameterMap& params) {
	double threshold = params["threshold"];
	return new SNRThresholdDecider(this, threshold, sensitivity, findHost()->getIndex(), coreDebug);
}
