/***************************************************************************
 * file:       DummyRoute.cc
 *
 * author:      Jerome Rousselot
 *
 * copyright:   (C) 2009 CSEM SA, Neuchatel, Switzerland.
 *
 * description: Adaptor module that simply "translates" netwControlInfo to macControlInfo
 *
 **************************************************************************/

#include <limits>
#include <algorithm>

#include "DummyRoute.h"
#include <cassert>

Define_Module(DummyRoute);

void DummyRoute::initialize(int stage) {
	BaseLayer::initialize(stage);
	if (stage == 0) {
		headerLength = par("headerLength");
		stats = par("stats");
		trace = par("trace");
		debug = par("debug");
	}
}


void DummyRoute::handleLowerMsg(cMessage* msg) {
	sendUp(msg);
}

void DummyRoute::handleLowerControl(cMessage *msg) {
	sendControlUp(msg);
}

void DummyRoute::handleUpperMsg(cMessage* msg) {
	NetwControlInfo* cInfo =
			dynamic_cast<NetwControlInfo*> (msg->removeControlInfo());
	int nextHopMacAddr;
	if (cInfo == 0) {
		EV<<"DummyRoute warning: Application layer did not specifiy a destination L3 address\n"
	       << "\tusing broadcast address instead\n";
		nextHopMacAddr = 0;
	} else {
		nextHopMacAddr = cInfo->getNetwAddr();
	}
	nextHopMacAddr = cInfo->getNetwAddr();
	msg->setControlInfo(new MacControlInfo(nextHopMacAddr));
	sendDown(msg);
}

void DummyRoute::finish() {
}

cMessage* DummyRoute::decapsMsg(NetwPkt *msg) {
}

