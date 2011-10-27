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
#include "WiseRoute.h"

#include <limits>
#include <algorithm>
#include <cassert>

#include "NetwControlInfo.h"
#include "MacToNetwControlInfo.h"
#include "ArpInterface.h"
#include "FindModule.h"
#include "WiseRoutePkt_m.h"
#include "SimTracer.h"
#include "ChannelAccess.h"

using std::make_pair;

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

		sinkAddress = LAddress::L3Type( par("sinkAddress").longValue() ); // 0
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
		  tracer = FindModule<SimTracer*>::findGlobalModule(); 
		  //const char *tracerModulePath = "sim.simTracer";
		  //cModule *modp = simulation.getModuleByPath(tracerModulePath);
		  //tracer = check_and_cast<SimTracer *>(modp);
		  if (!tracer) {
			error("No SimTracer module found, please check your ned configuration.");
		  }
		  // log node position
		  ChannelMobilityPtrType ptrMobility = ChannelMobilityAccessType().get();
		  if (ptrMobility) {
		    Coord pos = ptrMobility->getCurrentPosition();
		    tracer->logPosition(myNetwAddr, pos.x, pos.y, pos.z);
		  }
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
		WiseRoutePkt* pkt = new WiseRoutePkt("route-flood", ROUTE_FLOOD);
		pkt->setByteLength(headerLength);
		pkt->setInitialSrcAddr(myNetwAddr);
		pkt->setFinalDestAddr(LAddress::L3BROADCAST);
		pkt->setSrcAddr(myNetwAddr);
		pkt->setDestAddr(LAddress::L3BROADCAST);
		pkt->setNbHops(0);
		floodTable.insert(make_pair(myNetwAddr, floodSeqNumber));
		pkt->setSeqNum(floodSeqNumber);
		floodSeqNumber++;
		pkt->setIsFlood(1);
		setDownControlInfo(pkt, LAddress::L2BROADCAST);
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
	WiseRoutePkt*           netwMsg        = check_and_cast<WiseRoutePkt*>(msg);
	const LAddress::L3Type& finalDestAddr  = netwMsg->getFinalDestAddr();
	const LAddress::L3Type& initialSrcAddr = netwMsg->getInitialSrcAddr();
	const LAddress::L3Type& srcAddr        = netwMsg->getSrcAddr();
	double rssi = static_cast<MacToNetwControlInfo*>(netwMsg->getControlInfo())->getRSSI();
	double ber = static_cast<MacToNetwControlInfo*>(netwMsg->getControlInfo())->getBitErrorRate();
	// Check whether the message is a flood and if it has to be forwarded.
	floodTypes floodType = updateFloodTable(netwMsg->getIsFlood(), initialSrcAddr, finalDestAddr,
										    netwMsg->getSeqNum());
	if(trace) {
	  allReceivedRSSI.record(rssi);
	  allReceivedBER.record(ber);
	}
	if (floodType == DUPLICATE) {
		nbDuplicatedFloodsReceived++;
		delete netwMsg;
	}
	else {
		const cObject* pCtrlInfo = NULL;
		// If the message is a route flood, update the routing table.
		if (netwMsg->getKind() == ROUTE_FLOOD)
			updateRouteTable(initialSrcAddr, srcAddr, rssi, ber);

		if (finalDestAddr == myNetwAddr || LAddress::isL3Broadcast(finalDestAddr)) {
			WiseRoutePkt* msgCopy;
			if (floodType == FORWARD) {
				// it's a flood. copy for delivery, forward original.
				// if we are here (see updateFloodTable()), finalDestAddr == IP Broadcast. Hence finalDestAddr,
				// initialSrcAddr, and destAddr have already been correctly set
				// at origin, as well as the MAC control info. Hence only update
				// local hop source address.
				msgCopy = check_and_cast<WiseRoutePkt*>(netwMsg->dup());
				netwMsg->setSrcAddr(myNetwAddr);
				pCtrlInfo = netwMsg->removeControlInfo();
				setDownControlInfo(netwMsg, LAddress::L2BROADCAST);
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
				pCtrlInfo = netwMsg->removeControlInfo();
				setDownControlInfo(netwMsg, LAddress::L2BROADCAST);
				netwMsg->setNbHops(netwMsg->getNbHops()+1);
				sendDown(netwMsg);
				nbDataPacketsForwarded++;
				nbUnicastFloodForwarded++;
			}
			else {
				LAddress::L3Type nextHop = getRoute(finalDestAddr);
				if (LAddress::isL3Broadcast(nextHop)) {
					// no route exist to destination, attempt to send to final destination
					nextHop = finalDestAddr;
					nbGetRouteFailures++;
				}
				netwMsg->setSrcAddr(myNetwAddr);
				netwMsg->setDestAddr(nextHop);
				pCtrlInfo = netwMsg->removeControlInfo();
				setDownControlInfo(netwMsg, arp->getMacAddr(nextHop));
				netwMsg->setNbHops(netwMsg->getNbHops()+1);
				sendDown(netwMsg);
				nbDataPacketsForwarded++;
				nbPureUnicastForwarded++;
			}
		}
		if (pCtrlInfo != NULL)
			delete pCtrlInfo;
	}
}

void WiseRoute::handleLowerControl(cMessage *msg)
{
    delete msg;
}

void WiseRoute::handleUpperMsg(cMessage* msg)
{
	LAddress::L3Type finalDestAddr;
	LAddress::L3Type nextHopAddr;
	LAddress::L2Type nextHopMacAddr;
	WiseRoutePkt*    pkt   = new WiseRoutePkt(msg->getName(), DATA);
	cObject*         cInfo = msg->removeControlInfo();

	pkt->setByteLength(headerLength);

	if ( cInfo == NULL ) {
	    EV << "WiseRoute warning: Application layer did not specifiy a destination L3 address\n"
	       << "\tusing broadcast address instead\n";
	    finalDestAddr = LAddress::L3BROADCAST;
	}
	else {
		EV <<"WiseRoute: CInfo removed, netw addr="<< NetwControlInfo::getAddressFromControlInfo( cInfo ) <<endl;
		finalDestAddr = NetwControlInfo::getAddressFromControlInfo( cInfo );
		delete cInfo;
	}

	pkt->setFinalDestAddr(finalDestAddr);
	pkt->setInitialSrcAddr(myNetwAddr);
	pkt->setSrcAddr(myNetwAddr);
	pkt->setNbHops(0);

	if (LAddress::isL3Broadcast(finalDestAddr))
		nextHopAddr = LAddress::L3BROADCAST;
	else
		nextHopAddr = getRoute(finalDestAddr, true);
	pkt->setDestAddr(nextHopAddr);
	if (LAddress::isL3Broadcast(nextHopAddr)) {
		// it's a flood.
		nextHopMacAddr = LAddress::L2BROADCAST;
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
	setDownControlInfo(pkt, nextHopMacAddr);
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

void WiseRoute::updateRouteTable(const LAddress::L3Type& origin, const LAddress::L3Type& lastHop, double rssi, double ber)
{
	tRouteTable::iterator pos;

	pos = routeTable.find(origin);
	if(trace) {
	  receivedRSSI.record(rssi);
	  receivedBER.record(ber);
	}
	if (pos == routeTable.end()) {
		// A route towards origin does not exist yet. Insert the currently discovered one
		// only if the received RSSI is above the threshold.
		if (rssi > rssiThreshold) {
			tRouteTableEntry newEntry;

			// last hop from origin means next hop towards origin.
			newEntry.nextHop = lastHop;
			newEntry.rssi = rssi;
			if(trace) {
			  routeRSSI.record(rssi);
			  routeBER.record(ber);
			}
			routeTable.insert(make_pair(origin, newEntry));
			if(useSimTracer) {
			  tracer->logLink(myNetwAddr, lastHop);
			}
			nbRoutesRecorded++;
			if (origin == LAddress::L3NULL && trace) {
				nextHopSelectionForSink.record(static_cast<double>(lastHop));
			}
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
	setUpControlInfo(m, msg->getSrcAddr());
	nbHops = nbHops + msg->getNbHops();
	// delete the netw packet
	delete msg;
	return m;
}

WiseRoute::floodTypes WiseRoute::updateFloodTable(bool isFlood, const tFloodTable::key_type& srcAddr, const tFloodTable::key_type& destAddr, unsigned long seqNum)
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

WiseRoute::tFloodTable::key_type WiseRoute::getRoute(const tFloodTable::key_type& destAddr, bool iAmOrigin)
{
	// Find a route to dest address. As in the embedded code, if no route exists, indicate
	// final destination as next hop. If we'are lucky, final destination is one hop away...
	// If I am the origin of the packet and no route exists, use flood, hence return broadcast
	// address for next hop.
	tRouteTable::iterator pos = routeTable.find(destAddr);
	if (pos != routeTable.end())
		return pos->second.nextHop;
	else
		return LAddress::L3BROADCAST;
}
