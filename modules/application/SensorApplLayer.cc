/***************************************************************************
 * file:        SensorApplLayer.h
 *
 * author:      Amre El-Hoiydi, Jerome Rousselot
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

#define SINK_ADDR 0
#define ALTERNATIVE_ADDR 6
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

		EV<< "in initialize() stage 0...";
		debug = par("debug");
		stats = par("stats");
		nbPackets = par("nbPackets");
		trafficParam = par("trafficParam");
		initializationTime = par("initializationTime");
		broadcastPackets = par("broadcastPackets");
		headerLength = par("headerLength");
		// application configuration
		const char *traffic = par("trafficType");

		nbPacketsSent = 0;
		nbPacketsReceived = 0;
		firstPacketGeneration = -1;
		lastPacketReception = -2;

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
		delayTimer = new cMessage("appDelay", SEND_DATA_TIMER);
		// Blackboard stuff:
		hostID = getParentModule()->getId();

		// get pointer to the world module

		utility2 = FindModule<BaseUtility*>::findGlobalModule();
		catPacket = utility2->getCategory(&packet);

	} else if (stage == 1) {
		EV << "in initialize() stage 1...";
		// Application address configuration: equals to host IP address
		cModule *mac = getParentModule()->getSubmodule("nic")->getSubmodule("mac");


		myAppAddr = mac->par("netaddress");
		sentPackets = 0;

		// the sink does not generate packets to itself.
		//     if (myAppAddr != SINK_ADDR)		// change rso (it does)
		//scheduleNextPacket();
		// first packet generation time is always chosen uniformly
		// to avoid systematic collisions

		// schedule first packet with a uniform distribution to avoid systematic collisions
		if(nbPackets> 0)
		scheduleAt(simTime() +uniform(initializationTime, initializationTime + trafficParam), delayTimer);

		if (stats) {
			cModule *host = getParentModule();
			int nbNodes = host->size();
			latenciesRaw.setName("rawLatencies");
			for (int i = 0; i < nbNodes; i++) {
				std::ostringstream oss;
				oss << i;
				cStdDev aLatency(oss.str().c_str());
				latencies.push_back(aLatency);
			}
			latency.setName("latency");
		}
	}
}

void SensorApplLayer::scheduleNextPacket() {
	if (nbPackets > sentPackets && trafficType != 0) { // We must generate packets

		simtime_t waitTime = -1;

		switch (trafficType) {
		case PERIODIC:
			waitTime = trafficParam;
			EV<< "Periodic traffic, waitTime=" << waitTime << endl;
			break;
			case UNIFORM:
			waitTime = uniform(0, trafficParam);
			EV << "Uniform traffic, waitTime=" << waitTime << endl;
			break;
			case EXPONENTIAL:
			waitTime = exponential(trafficParam);
			EV << "Exponential traffic, waitTime=" << waitTime << endl;
			break;
			case UNKNOWN:
			default:
			EV <<
			"Cannot generate requested traffic type (unimplemented or unknown)."
			<< endl;

		}
		EV << "Start timer for a new packet in " << waitTime << " seconds." <<
		endl;
		//drop(delayTimer);
		//delete delayTimer;
		//delayTimer = new cMessage( "delay-timer", SEND_DATA_TIMER );
		scheduleAt(simTime() + waitTime, delayTimer);
		EV << "...timer rescheduled." << endl;
	} else {
		EV << "All packets sent.\n";
	}
}

			/**
			 * Handling of messages arrived to destination
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
		utility2->publishBBItem(catPacket, &packet, hostID);
		// EV << "Received a data packet from host["<<m->getSrcAddr()<<"]\n";
		if (stats) {
			//                      cStdDev latency = latencies[m->getSrcAddr()];
			//                      latency.collect(m->getArrivalTime()-m->getCreationTime());
			//                      testStat.collect(m->getArrivalTime()-m->getCreationTime());
			//                      EV << "Received a data packet from host["<<m->getSrcAddr()<<"], latency=" <<  m->getArrivalTime()-m->getCreationTime() << ", collected " << latency.getCount() << "mean is now: " << latency.getMean() << endl;
			simtime_t theLatency = m->getArrivalTime() - m->getCreationTime();
			latencies[m->getSrcAddr()].collect(theLatency);
			latency.collect(theLatency);
			if (firstPacketGeneration < 0)
				firstPacketGeneration = m->getCreationTime();
			lastPacketReception = m->getArrivalTime();
			EV<< "Received a data packet from host[" << m->getSrcAddr()
			<< "], latency=" << theLatency
			<< ", collected " << latencies[m->getSrcAddr()].
			getCount() << "mean is now: " << latencies[m->getSrcAddr()].
			getMean() << endl;
			latenciesRaw.record(theLatency.dbl());
		}
		delete msg;

		//  sendReply(m);
		break;
		default:
		EV << "Error! got packet with unknown kind: " << msg->getKind() << endl;
		delete msg;
	}
}

			/**
			 * A timer with kind = SEND_DATA_TIMER indicates that a new
			 * data has to be send (@ref sendData).
			 *
			 * There are no other timer implemented for this module.
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
	}
}

void SensorApplLayer::handleLowerControl(cMessage * msg) {
	delete msg;
}
		/**
		 * This function creates a new data message and sends it down to
		 * the network layer
		 **/
void SensorApplLayer::sendData() {
	ApplPkt *pkt = new ApplPkt("Data", DATA_MESSAGE);

	if(broadcastPackets) {
		pkt->setDestAddr(NET_BROADCAST);
	} else if (myAppAddr == SINK_ADDR) {
		pkt->setDestAddr(ALTERNATIVE_ADDR);
	} else {
		pkt->setDestAddr(SINK_ADDR);
	}
	// we use the host modules getIndex() as a appl address
	pkt->setSrcAddr(myAppAddr);
	pkt->setByteLength(headerLength);
	// set the control info to tell the network layer the layer 3
	// address;
	pkt->setControlInfo(new NetwControlInfo(pkt->getDestAddr()));
	EV<< "Sending data packet!\n";
	sendDown(pkt);
	//send(pkt, dataOut);
	nbPacketsSent++;
	packet.setPacketSent(true);
	packet.setNbPacketsSent(1);
	packet.setNbPacketsReceived(0);
	packet.setHost(myAppAddr);
	utility2->publishBBItem(catPacket, &packet, hostID);
	sentPackets++;
	scheduleNextPacket();
}

void SensorApplLayer::finish() {
	if (stats) {
		// output logs to scalar file

		for (unsigned int i = 0; i < latencies.size(); i++) {
			char dispstring[12];
			cStdDev aLatency = latencies[i];

			//EV << "Recording mean latency for node " << i << ": " << aLatency.getMean() << endl;
			//recordScalar("mean_latency ", aLatency.getMean());
			sprintf(dispstring, "latency%d", i);
			//dispstring
			recordScalar(dispstring, aLatency.getMean());
			aLatency.record();
		}
		recordScalar("activity duration", lastPacketReception
				- firstPacketGeneration);
		recordScalar("firstPacketGeneration", firstPacketGeneration);
		recordScalar("lastPacketReception", lastPacketReception);
		recordScalar("nbPacketsSent", nbPacketsSent);
		recordScalar("nbPacketsReceived", nbPacketsReceived);
		latency.record();
	}
	BaseModule::finish();
}

SensorApplLayer::~SensorApplLayer() {
	cancelAndDelete(delayTimer);
}
