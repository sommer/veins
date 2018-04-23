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

#include "veins/modules/phy/PhyLayer80211p.h"

#include "veins/modules/phy/Decider80211p.h"
#include "veins/modules/analogueModel/SimplePathlossModel.h"
#include "veins/modules/analogueModel/BreakpointPathlossModel.h"
#include "veins/modules/analogueModel/LogNormalShadowing.h"
#include "veins/modules/analogueModel/JakesFading.h"
#include "veins/modules/analogueModel/PERModel.h"
#include "veins/modules/analogueModel/SimpleObstacleShadowing.h"
#include "veins/modules/analogueModel/TwoRayInterferenceModel.h"
#include "veins/modules/analogueModel/NakagamiFading.h"
#include "veins/base/connectionManager/BaseConnectionManager.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/messages/AirFrame11p_m.h"
#include "veins/base/phyLayer/MacToPhyControlInfo.h"

using Veins::ObstacleControlAccess;

Define_Module(PhyLayer80211p);

/** This is needed to circumvent a bug in MiXiM that allows different header length interpretations for receiving and sending airframes*/
void PhyLayer80211p::initialize(int stage) {
	if (stage == 0) {
		//get ccaThreshold before calling BasePhyLayer::initialize() which instantiates the deciders
		ccaThreshold = pow(10, par("ccaThreshold").doubleValue() / 10);
		allowTxDuringRx = par("allowTxDuringRx").boolValue();
		collectCollisionStatistics = par("collectCollisionStatistics").boolValue();
	}
	BasePhyLayer::initialize(stage);
	if (stage == 0) {
		if (par("headerLength").longValue() != PHY_HDR_TOTAL_LENGTH) {
		throw cRuntimeError("The header length of the 802.11p standard is 46bit, please change your omnetpp.ini accordingly by either setting it to 46bit or removing the entry");
		}
		//erase the RadioStateAnalogueModel
		analogueModels.erase(analogueModels.begin());
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
		if (world->use2D()) error("The TwoRayInterferenceModel uses nodes' z-position as the antenna height over ground. Refusing to work in a 2D world");
		return initializeTwoRayInterferenceModel(params);
	}
	else if (name == "NakagamiFading")
	{
		return initializeNakagamiFading(params);
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
			throw cRuntimeError("TestPhyLayer::createPathLossModel(): alpha can't be smaller than specified in \
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
			throw cRuntimeError("TestPhyLayer::createPathLossModel(): alpha can't be smaller than specified in \
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
			throw cRuntimeError("TestPhyLayer::createPathLossModel(): carrierFrequency can't be smaller than specified in \
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
		throw cRuntimeError("Undefined parameters for breakpointPathlossModel. Please check your configuration.");
	}

	return new BreakpointPathlossModel(L01, L02, alpha1, alpha2, breakpointDistance, carrierFrequency, useTorus, playgroundSize, coreDebug);

}

AnalogueModel* PhyLayer80211p::initializeTwoRayInterferenceModel(ParameterMap& params) {
	ASSERT(params.count("DielectricConstant") == 1);

	double dielectricConstant= params["DielectricConstant"].doubleValue();

	return new TwoRayInterferenceModel(dielectricConstant, coreDebug);
}

AnalogueModel* PhyLayer80211p::initializeNakagamiFading(ParameterMap& params) {
	bool constM = params["constM"].boolValue();
	double m = 0;
	if (constM) {
		m = params["m"].doubleValue();
	}
	return new NakagamiFading(constM, m, coreDebug);
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
			throw cRuntimeError("TestPhyLayer::createPathLossModel(): alpha can't be smaller than specified in \
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
			throw cRuntimeError("TestPhyLayer::createPathLossModel(): carrierFrequency can't be smaller than specified in \
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
			throw cRuntimeError("initializeSimpleObstacleShadowing(): carrierFrequency can't be smaller than specified in ConnectionManager. Please adjust your config.xml file accordingly");
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
	if (!obstacleControlP) throw cRuntimeError("initializeSimpleObstacleShadowing(): cannot find ObstacleControl module");
	return new SimpleObstacleShadowing(*obstacleControlP, carrierFrequency, useTorus, playgroundSize, coreDebug);
}

Decider* PhyLayer80211p::initializeDecider80211p(ParameterMap& params) {
	double centerFreq = params["centerFrequency"];
	Decider80211p* dec = new Decider80211p(this, sensitivity, ccaThreshold, allowTxDuringRx, centerFreq, findHost()->getIndex(), collectCollisionStatistics, coreDebug);
	dec->setPath(getParentModule()->getFullPath());
	return dec;
}

void PhyLayer80211p::changeListeningFrequency(double freq) {
	Decider80211p* dec = dynamic_cast<Decider80211p*>(decider);
	assert(dec);
	dec->changeFrequency(freq);
}
void PhyLayer80211p::handleSelfMessage(cMessage* msg) {

	switch(msg->getKind()) {
	//transmission overBasePhyLayer::
	case TX_OVER: {
		assert(msg == txOverTimer);
		sendControlMsgToMac(new cMessage("Transmission over", TX_OVER));
		//check if there is another packet on the chan, and change the chan-state to idle
		Decider80211p* dec = dynamic_cast<Decider80211p*>(decider);
		assert(dec);
		if (dec->cca(simTime(),NULL)) {
			//chan is idle
			DBG << "Channel idle after transmit!\n";
			dec->setChannelIdleStatus(true);

		}
		else {
			DBG << "Channel not yet idle after transmit!\n";
		}
		break;
	}
	//radio switch over
	case RADIO_SWITCHING_OVER:
		assert(msg == radioSwitchingOverTimer);
		BasePhyLayer::finishRadioSwitching();
		break;

	//AirFrame
	case AIR_FRAME:
		BasePhyLayer::handleAirFrame(static_cast<AirFrame*>(msg));
		break;

	//ChannelSenseRequest
	case CHANNEL_SENSE_REQUEST:
		BasePhyLayer::handleChannelSenseRequest(msg);
		break;

	default:
		break;
	}
}
AirFrame *PhyLayer80211p::encapsMsg(cPacket *macPkt)
{
	// the cMessage passed must be a MacPacket... but no cast needed here
	// MacPkt* pkt = static_cast<MacPkt*>(msg);

	// ...and must always have a ControlInfo attached (contains Signal)
	cObject* ctrlInfo = macPkt->removeControlInfo();
	assert(ctrlInfo);

	// create the new AirFrame
	AirFrame* frame = new AirFrame11p(macPkt->getName(), AIR_FRAME);

	// Retrieve the pointer to the Signal-instance from the ControlInfo-instance.
	// We are now the new owner of this instance.
	Signal* s = MacToPhyControlInfo::getSignalFromControlInfo(ctrlInfo);
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
	frame->setSchedulingPriority(airFramePriority());
	frame->setProtocolId(myProtocolId());
	frame->setBitLength(headerLength);
	frame->setId(world->getUniqueAirFrameId());
	frame->setChannel(radio->getCurrentChannel());


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
int PhyLayer80211p::getRadioState() {
	return BasePhyLayer::getRadioState();
};

simtime_t PhyLayer80211p::setRadioState(int rs) {
	if (rs == Radio::TX)
		decider->switchToTx();
	return BasePhyLayer::setRadioState(rs);
}

void PhyLayer80211p::setCCAThreshold(double ccaThreshold_dBm) {
	ccaThreshold = pow(10, ccaThreshold_dBm / 10);
	((Decider80211p *)decider)->setCCAThreshold(ccaThreshold_dBm);
}
double PhyLayer80211p::getCCAThreshold() {
	return 10 * log10(ccaThreshold);
}

void PhyLayer80211p::notifyMacAboutRxStart(bool enable) {
    ((Decider80211p *)decider)->setNotifyRxStart(enable);
}

void PhyLayer80211p::requestChannelStatusIfIdle() {
    Enter_Method_Silent();
    Decider80211p* dec = (Decider80211p*)decider;
    if (dec->cca(simTime(),NULL)) {
        //chan is idle
        DBG << "Request channel status: channel idle!\n";
        dec->setChannelIdleStatus(true);
    }
}
