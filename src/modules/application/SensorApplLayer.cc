/***************************************************************************
 * file:        SensorApplLayer.h
 *
 * author:      Amre El-Hoiydi, Jerome Rousselot, Ramon Serna Oliver
 *
 * copyright:   (C) 2007-2008 CSEM
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: Generate periodic traffic addressed to a sink
 **************************************************************************/

#include "SensorApplLayer.h"

#include <sstream>

#include "BaseNetwLayer.h"
#include "AddressingInterface.h"
#include "NetwControlInfo.h"
#include "FindModule.h"
#include "SimpleAddress.h"
#include "BaseWorldUtility.h"
#include "ApplPkt_m.h"

Define_Module(SensorApplLayer);

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BasicModule.
 *
 * Then we will set a timer to indicate the first time we will send a
 * message
 *
 **/
void SensorApplLayer::initialize(int stage) {
    BaseLayer::initialize(stage);
	if (stage == 0) {

		debugEV<< "in initialize() stage 0...";
		debug = par("debug");
		stats = par("stats");
		trace = par("trace");
		nbPackets = par("nbPackets");
		trafficParam = par("trafficParam");
		initializationTime = par("initializationTime");
		broadcastPackets = par("broadcastPackets");
		headerLength = par("headerLength");
		// application configuration
		const char *traffic = par("trafficType");
		destAddr = LAddress::L3Type(par("destAddr").longValue());
		nbPacketsSent = 0;
		nbPacketsReceived = 0;
		firstPacketGeneration = -1;
		lastPacketReception = -2;

		initializeDistribution(traffic);

		delayTimer = new cMessage("appDelay", SEND_DATA_TIMER);

		// get pointer to the world module
		world = FindModule<BaseWorldUtility*>::findGlobalModule();

	} else if (stage == 1) {
		debugEV << "in initialize() stage 1...";
		// Application address configuration: equals to host address

		cModule *const pHost = findHost();
		const cModule* netw  = FindModule<BaseNetwLayer*>::findSubModule(pHost);
		if(!netw) {
			netw = pHost->getSubmodule("netwl");
			if(!netw) {
				opp_error("Could not find network layer module. This means "
						  "either no network layer module is present or the "
						  "used network layer module does not subclass from "
						  "BaseNetworkLayer.");
			}
		}
		const AddressingInterface *const addrScheme = FindModule<AddressingInterface*>::findSubModule(pHost);
		if(addrScheme) {
			myAppAddr = addrScheme->myNetwAddr(netw);
		} else {
			myAppAddr = LAddress::L3Type( netw->getId() );
		}
		sentPackets = 0;

		// first packet generation time is always chosen uniformly
		// to avoid systematic collisions
		if(nbPackets> 0)
			scheduleAt(simTime() +uniform(initializationTime, initializationTime + trafficParam), delayTimer);

		if (stats) {
			latenciesRaw.setName("rawLatencies");
			latenciesRaw.setUnit("s");
			latency.setName("latency");
		}
	}
}

cStdDev& SensorApplLayer::hostsLatency(const LAddress::L3Type& hostAddress)
{
	using std::pair;

	if(latencies.count(hostAddress) == 0) {
		std::ostringstream oss;
		oss << hostAddress;
		cStdDev aLatency(oss.str().c_str());
		latencies.insert(pair<LAddress::L3Type, cStdDev>(hostAddress, aLatency));
	}

	return latencies[hostAddress];
}

void SensorApplLayer::initializeDistribution(const char* traffic) {
	if (!strcmp(traffic, "periodic")) {
		trafficType = PERIODIC;
	} else if (!strcmp(traffic, "uniform")) {
		trafficType = UNIFORM;
	} else if (!strcmp(traffic, "exponential")) {
		trafficType = EXPONENTIAL;
	} else {
		trafficType = UNKNOWN;
		EV << "Error! Unknown traffic type: " << traffic << endl;
	}
}

void SensorApplLayer::scheduleNextPacket() {
	if (nbPackets > sentPackets && trafficType != 0) { // We must generate packets

		simtime_t waitTime = SIMTIME_ZERO;

		switch (trafficType) {
		case PERIODIC:
			waitTime = trafficParam;
			debugEV<< "Periodic traffic, waitTime=" << waitTime << endl;
			break;
			case UNIFORM:
			waitTime = uniform(0, trafficParam);
			debugEV << "Uniform traffic, waitTime=" << waitTime << endl;
			break;
			case EXPONENTIAL:
			waitTime = exponential(trafficParam);
			debugEV << "Exponential traffic, waitTime=" << waitTime << endl;
			break;
			case UNKNOWN:
			default:
			EV <<
			"Cannot generate requested traffic type (unimplemented or unknown)."
			<< endl;
			return; // don not schedule
			break;
		}
		debugEV << "Start timer for a new packet in " << waitTime << " seconds." <<
		endl;
		scheduleAt(simTime() + waitTime, delayTimer);
		debugEV << "...timer rescheduled." << endl;
	} else {
		debugEV << "All packets sent.\n";
	}
}

/**
 * @brief Handling of messages arrived to destination
 **/
void SensorApplLayer::handleLowerMsg(cMessage * msg) {
	ApplPkt *m;

	switch (msg->getKind()) {
	case DATA_MESSAGE:
		m = static_cast<ApplPkt *> (msg);
		nbPacketsReceived++;
		packet.setPacketSent(false);
		packet.setNbPacketsSent(0);
		packet.setNbPacketsReceived(1);
		packet.setHost(myAppAddr);
		emit(BaseLayer::catPacketSignal, &packet);
		if (stats) {
			simtime_t theLatency = m->getArrivalTime() - m->getCreationTime();
			if(trace) {
			  hostsLatency(m->getSrcAddr()).collect(theLatency);
			  latenciesRaw.record(SIMTIME_DBL(theLatency));
			}
			latency.collect(theLatency);
			if (firstPacketGeneration < 0)
				firstPacketGeneration = m->getCreationTime();
			lastPacketReception = m->getArrivalTime();
			if(trace) {
			  debugEV<< "Received a data packet from host[" << m->getSrcAddr()
			  << "], latency=" << theLatency
			  << ", collected " << hostsLatency(m->getSrcAddr()).
			  getCount() << "mean is now: " << hostsLatency(m->getSrcAddr()).
			  getMean() << endl;
			} else {
				  debugEV<< "Received a data packet from host[" << m->getSrcAddr()
				  << "], latency=" << theLatency << endl;
			}
		}
		delete msg;

		//  sendReply(m);
		break;
		default:
		EV << "Error! got packet with unknown kind: " << msg->getKind() << endl;
		delete msg;
		break;
	}
}

/**
 * @brief A timer with kind = SEND_DATA_TIMER indicates that a new
 * data has to be send (@ref sendData).
 *
 * There are no other timers implemented for this module.
 *
 * @sa sendData
 **/
void SensorApplLayer::handleSelfMsg(cMessage * msg) {
	switch (msg->getKind()) {
	case SEND_DATA_TIMER:
		sendData();
		//delete msg;
		break;
	default:
		EV<< "Unkown selfmessage! -> delete, kind: " << msg->getKind() << endl;
		delete msg;
		break;
	}
}

void SensorApplLayer::handleLowerControl(cMessage * msg) {
	delete msg;
}

/**
  * @brief This function creates a new data message and sends it down to
  * the network layer
 **/
void SensorApplLayer::sendData() {
	ApplPkt *pkt = new ApplPkt("Data", DATA_MESSAGE);

	if(broadcastPackets) {
		pkt->setDestAddr(LAddress::L3BROADCAST);
	} else {
		pkt->setDestAddr(destAddr);
	}
	pkt->setSrcAddr(myAppAddr);
	pkt->setByteLength(headerLength);
	// set the control info to tell the network layer the layer 3 address
	NetwControlInfo::setControlInfo(pkt, pkt->getDestAddr());
	debugEV<< "Sending data packet!\n";
	sendDown(pkt);
	nbPacketsSent++;
	packet.setPacketSent(true);
	packet.setNbPacketsSent(1);
	packet.setNbPacketsReceived(0);
	packet.setHost(myAppAddr);
	emit(BaseLayer::catPacketSignal, &packet);
	sentPackets++;
	scheduleNextPacket();
}

void SensorApplLayer::finish() {
	using std::map;
	if (stats) {
		if (trace) {
			std::stringstream osToStr(std::stringstream::out);
			// output logs to scalar file
			for (map<LAddress::L3Type, cStdDev>::iterator it = latencies.begin(); it != latencies.end(); ++it) {
				cStdDev aLatency = it->second;

				osToStr.str(""); osToStr << "latency" << it->first;
				recordScalar(osToStr.str().c_str(), aLatency.getMean(), "s");
				aLatency.record();
			}
		}
		recordScalar("activity duration", lastPacketReception
				- firstPacketGeneration, "s");
		recordScalar("firstPacketGeneration", firstPacketGeneration, "s");
		recordScalar("lastPacketReception", lastPacketReception, "s");
		recordScalar("nbPacketsSent", nbPacketsSent);
		recordScalar("nbPacketsReceived", nbPacketsReceived);
		latency.record();
	}
	cComponent::finish();
}

SensorApplLayer::~SensorApplLayer() {
	cancelAndDelete(delayTimer);
}
