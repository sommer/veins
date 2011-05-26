/***************************************************************************
 * file:        WiseRoute.cc
 *
 * author:      Damien Piguet, Jerome Rousselot
 *
 * copyright:   (C) 2007-2008 CSEM SA, Neuchatel, Switzerland.
 *
 * description: Implementation of the routing protocol of WiseStack.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * ported to Mixim 2.0.1 by Theodoros Kapourniotis
 * last modification: 06/02/11
 **************************************************************************/

#include <limits>
#include <algorithm>

#include "WiseRoute.h"
#include <cassert>

Define_Module(WiseRoute);

void WiseRoute::initialize(int stage)
{
	BaseNetwLayer::initialize(stage);

	if(stage == 1) {

		EV << "Host index=" << findHost()->getIndex() << ", Id="
		<< findHost()->getId() << endl;


		EV << "  host IP address=" << myNetwAddr << endl;
		EV << "  host macaddress=" << arp->getMacAddr(myNetwAddr) << endl;
		macaddress = arp->getMacAddr(myNetwAddr);

		sinkAddress = par("sinkAddress"); // 0
		headerLength = par ("headerLength");
		rssiThreshold = par("rssiThreshold");
		routeFloodsInterval = par("routeFloodsInterval");

		stats = par("stats");
		trace = par("trace");
		debug = par("debug");
		useSimTracer = par("useSimTracer");
		floodSeqNumber = 0;

		nbDataPacketsForwarded = 0;
		nbDataPacketsReceived = 0;
		nbDataPacketsSent = 0;
		nbDuplicatedFloodsReceived = 0;
		nbFloodsSent = 0;
		nbPureUnicastSent = 0;
		nbRouteFloodsSent = 0;
		nbRouteFloodsReceived = 0;
		nbUnicastFloodForwarded = 0;
		nbPureUnicastForwarded = 0;
		nbGetRouteFailures = 0;
		nbRoutesRecorded = 0;
		nbHops = 0;
		receivedRSSI.setName("receivedRSSI");
		routeRSSI.setName("routeRSSI");
		allReceivedRSSI.setName("allReceivedRSSI");
		receivedBER.setName("receivedBER");
		routeBER.setName("routeBER");
		allReceivedBER.setName("allReceivedBER");
		nextHopSelectionForSink.setName("nextHopSelectionForSink");

		routeFloodTimer = new cMessage("route-flood-timer", SEND_ROUTE_FLOOD_TIMER);
		// only schedule a flood of the node is a sink!!
		if (routeFloodsInterval > 0 && myNetwAddr==sinkAddress)
			scheduleAt(simTime() + uniform(0.5, 1.5), routeFloodTimer);

		if(useSimTracer) {
		  // Get a handle to the tracer module
		  const char *tracerModulePath = "sim.simTracer";
		  cModule *modp = simulation.getModuleByPath(tracerModulePath);
		  tracer = check_and_cast<SimTracer *>(modp);
		}
		// log node position
		BaseMobility * mobility;
		mobility = check_and_cast<BaseMobility*>(getParentModule()->getSubmodule("mobility"));
		if(useSimTracer) {
//		  tracer->logPosition(myNetwAddr, mobility->getX(), mobility->getY());
		}
	}
}

WiseRoute::~WiseRoute()
{
	cancelAndDelete(routeFloodTimer);
}

void WiseRoute::handleSelfMsg(cMessage* msg)
{
	if (msg->getKind() == SEND_ROUTE_FLOOD_TIMER) {
		// Send route flood packet and restart the timer
		int macBcastAddr = L2BROADCAST;
		int ipBcastAddr = L3BROADCAST;
		WiseRoutePkt* pkt = new WiseRoutePkt("route-flood", ROUTE_FLOOD);
		pkt->setByteLength(headerLength);
		pkt->setInitialSrcAddr(myNetwAddr);
		pkt->setFinalDestAddr(ipBcastAddr);
		pkt->setSrcAddr(myNetwAddr);
		pkt->setDestAddr(ipBcastAddr);
		pkt->setNbHops(0);
		floodTable.insert(make_pair(myNetwAddr, floodSeqNumber));
		pkt->setSeqNum(floodSeqNumber);
		floodSeqNumber++;
		pkt->setIsFlood(1);
		pkt->setControlInfo(new NetwToMacControlInfo(macBcastAddr));
		sendDown(pkt);
		nbFloodsSent++;
		nbRouteFloodsSent++;
		scheduleAt(simTime() + routeFloodsInterval + uniform(0, 1), routeFloodTimer);
	}
	else {
		EV << "WiseRoute - handleSelfMessage: got unexpected message of kind " << msg->getKind() << endl;
		delete msg;
	}
}


void WiseRoute::handleLowerMsg(cMessage* msg)
{
	int macBcastAddr = L3BROADCAST;
	int bcastIpAddr = L2BROADCAST;
	WiseRoutePkt* netwMsg = check_and_cast<WiseRoutePkt*>(msg);
	int finalDestAddr = netwMsg->getFinalDestAddr();
	int initialSrcAddr = netwMsg->getInitialSrcAddr();
	int srcAddr = netwMsg->getSrcAddr();
	double rssi = static_cast<MacToNetwControlInfo*>(netwMsg->getControlInfo())->getRSSI();
	double ber = static_cast<MacToNetwControlInfo*>(netwMsg->getControlInfo())->getBitErrorRate();
	// Check whether the message is a flood and if it has to be forwarded.
	floodTypes floodType = updateFloodTable(netwMsg->getIsFlood(), initialSrcAddr, finalDestAddr,
										    netwMsg->getSeqNum());
	allReceivedRSSI.record(rssi);
	allReceivedBER.record(ber);
	if (floodType == DUPLICATE) {
		nbDuplicatedFloodsReceived++;
		delete netwMsg;
	}
	else {
		// If the message is a route flood, update the routing table.
		if (netwMsg->getKind() == ROUTE_FLOOD)
			updateRouteTable(initialSrcAddr, srcAddr, rssi, ber);

		if (finalDestAddr == myNetwAddr || finalDestAddr == bcastIpAddr) {
			WiseRoutePkt* msgCopy;
			if (floodType == FORWARD) {
				// it's a flood. copy for delivery, forward original.
				// if we are here (see updateFloodTable()), finalDestAddr == IP Broadcast. Hence finalDestAddr,
				// initialSrcAddr, and destAddr have already been correctly set
				// at origin, as well as the MAC control info. Hence only update
				// local hop source address.
				msgCopy = check_and_cast<WiseRoutePkt*>(netwMsg->dup());
				netwMsg->setSrcAddr(myNetwAddr);
//				((NetwToMacControlInfo*) netwMsg->getControlInfo())->setNextHopMac(macBcastAddr);
				netwMsg->removeControlInfo();
				netwMsg->setControlInfo(new NetwToMacControlInfo(macBcastAddr));
				netwMsg->setNbHops(netwMsg->getNbHops()+1);
				sendDown(netwMsg);
				nbDataPacketsForwarded++;
			}
			else
				msgCopy = netwMsg;
			if (msgCopy->getKind() == DATA) {
				sendUp(decapsMsg(msgCopy));
				nbDataPacketsReceived++;
			}
			else {
				nbRouteFloodsReceived++;
				delete msgCopy;
			}
		}
		else {
			// not for me. if flood, forward as flood. else select a route
			if (floodType == FORWARD) {
				netwMsg->setSrcAddr(myNetwAddr);
//				((NetwToMacControlInfo*) netwMsg->getControlInfo())->setNextHopMac(macBcastAddr);
				netwMsg->removeControlInfo();
				netwMsg->setControlInfo(new NetwToMacControlInfo(macBcastAddr));
				netwMsg->setNbHops(netwMsg->getNbHops()+1);
				sendDown(netwMsg);
				nbDataPacketsForwarded++;
				nbUnicastFloodForwarded++;
			}
			else {
				int nextHop = getRoute(finalDestAddr);
				if (nextHop == bcastIpAddr) {
					// no route exist to destination, attempt to send to final destination
					nextHop = finalDestAddr;
					nbGetRouteFailures++;
				}
				netwMsg->setSrcAddr(myNetwAddr);
				netwMsg->setDestAddr(nextHop);
//				((NetwToMacControlInfo*) netwMsg->getControlInfo())->setNextHopMac(arp->getMacAddr(nextHop));
				netwMsg->removeControlInfo();
				netwMsg->setControlInfo(new NetwToMacControlInfo(arp->getMacAddr(nextHop)));
				netwMsg->setNbHops(netwMsg->getNbHops()+1);
				sendDown(netwMsg);
				nbDataPacketsForwarded++;
				nbPureUnicastForwarded++;
			}
		}
	}
}

void WiseRoute::handleLowerControl(cMessage *msg)
{
    delete msg;
}

void WiseRoute::handleUpperMsg(cMessage* msg)
{
	int finalDestAddr;
	int nextHopAddr;
	unsigned long nextHopMacAddr;
	WiseRoutePkt* pkt = new WiseRoutePkt(msg->getName(), DATA);
	NetwControlInfo* cInfo = dynamic_cast<NetwControlInfo*>(msg->removeControlInfo());
	int ipBcastAddr = L3BROADCAST;

	pkt->setByteLength(headerLength);

	if ( cInfo == 0 ) {
	    EV << "WiseRoute warning: Application layer did not specifiy a destination L3 address\n"
	       << "\tusing broadcast address instead\n";
	    finalDestAddr = ipBcastAddr;
	}
	else {
		EV <<"WiseRoute: CInfo removed, netw addr="<< cInfo->getNetwAddr() <<endl;
		finalDestAddr = cInfo->getNetwAddr();
		delete cInfo;
	}

	pkt->setFinalDestAddr(finalDestAddr);
	pkt->setInitialSrcAddr(myNetwAddr);
	pkt->setSrcAddr(myNetwAddr);
	pkt->setNbHops(0);

	if (finalDestAddr == ipBcastAddr)
		nextHopAddr = ipBcastAddr;
	else
		nextHopAddr = getRoute(finalDestAddr, true);
	pkt->setDestAddr(nextHopAddr);
	if (nextHopAddr == ipBcastAddr) {
		// it's a flood.
		nextHopMacAddr = L2BROADCAST;
		pkt->setIsFlood(1);
		nbFloodsSent++;
		// record flood in flood table
		floodTable.insert(make_pair(myNetwAddr, floodSeqNumber));
		pkt->setSeqNum(floodSeqNumber);
		floodSeqNumber++;
		nbGetRouteFailures++;
	}
	else {
		pkt->setIsFlood(0);
		nbPureUnicastSent++;
		nextHopMacAddr = arp->getMacAddr(nextHopAddr);
	}
	pkt->setControlInfo(new NetwToMacControlInfo(nextHopMacAddr));
	assert(static_cast<cPacket*>(msg));
	pkt->encapsulate(static_cast<cPacket*>(msg));
	sendDown(pkt);
	nbDataPacketsSent++;
}

void WiseRoute::finish()
{
	if (stats) {
		recordScalar("nbDataPacketsForwarded", nbDataPacketsForwarded);
		recordScalar("nbDataPacketsReceived", nbDataPacketsReceived);
		recordScalar("nbDataPacketsSent", nbDataPacketsSent);
		recordScalar("nbDuplicatedFloodsReceived", nbDuplicatedFloodsReceived);
		recordScalar("nbFloodsSent", nbFloodsSent);
		recordScalar("nbPureUnicastSent", nbPureUnicastSent);
		recordScalar("nbRouteFloodsSent", nbRouteFloodsSent);
		recordScalar("nbRouteFloodsReceived", nbRouteFloodsReceived);
		recordScalar("nbUnicastFloodForwarded", nbUnicastFloodForwarded);
		recordScalar("nbPureUnicastForwarded", nbPureUnicastForwarded);
		recordScalar("nbGetRouteFailures", nbGetRouteFailures);
		recordScalar("nbRoutesRecorded", nbRoutesRecorded);
		recordScalar("meanNbHops", (double) nbHops / (double) nbDataPacketsReceived);
	}
}

void WiseRoute::updateRouteTable(int origin, int lastHop, double rssi, double ber)
{
	tRouteTable::iterator pos;

	pos = routeTable.find(origin);
	receivedRSSI.record(rssi);
	receivedBER.record(ber);
	if (pos == routeTable.end()) {
		// A route towards origin does not exist yet. Insert the currently discovered one
		// only if the received RSSI is above the threshold.
		if (rssi > rssiThreshold) {
			tRouteTableEntry newEntry;

			// last hop from origin means next hop towards origin.
			newEntry.nextHop = lastHop;
			newEntry.rssi = rssi;
			routeRSSI.record(rssi);
			routeBER.record(ber);
			routeTable.insert(make_pair(origin, newEntry));
			if(useSimTracer) {
			  tracer->logLink(myNetwAddr, lastHop);
			}
			nbRoutesRecorded++;
			if (origin == 0)
				nextHopSelectionForSink.record(lastHop);
		}
	}
	else {
		// A route towards the node which originated the received packet already exists.
		// Replace its entry only if the route proposal that we just received has a stronger
		// RSSI.
//		tRouteTableEntry entry = pos->second;
//		if (entry.rssi > rssiThreshold) {
//			entry.nextHop = lastHop;
//			entry.rssi = rssi;
//			if (origin == 0)
//				nextHopSelectionForSink.record(lastHop);
//		}
	}
}

cMessage* WiseRoute::decapsMsg(WiseRoutePkt *msg)
{
	cMessage *m = msg->decapsulate();
	m->setControlInfo(new NetwControlInfo(msg->getSrcAddr()));
	nbHops = nbHops + msg->getNbHops();
	// delete the netw packet
	delete msg;
	return m;
}

WiseRoute::floodTypes WiseRoute::updateFloodTable(bool isFlood, int srcAddr, int destAddr, unsigned long seqNum)
{
	if (isFlood) {
		tFloodTable::iterator pos = floodTable.lower_bound(srcAddr);
		tFloodTable::iterator posEnd = floodTable.upper_bound(srcAddr);

		while (pos != posEnd) {
			if (seqNum == pos->second)
				return DUPLICATE;  // this flood is known, don't forward it.
			++pos;
		}
		floodTable.insert(make_pair(srcAddr, seqNum));
		if (destAddr == myNetwAddr)
			return FORME;
		else
			return FORWARD;
	}
	else
		return NOTAFLOOD;
}

int WiseRoute::getRoute(int destAddr, bool iAmOrigin)
{
	// Find a route to dest address. As in the embedded code, if no route exists, indicate
	// final destination as next hop. If we'are lucky, final destination is one hop away...
	// If I am the origin of the packet and no route exists, use flood, hence return broadcast
	// address for next hop.
	tRouteTable::iterator pos = routeTable.find(destAddr);
	if (pos != routeTable.end())
		return pos->second.nextHop;
	else
		return L3BROADCAST;
}
