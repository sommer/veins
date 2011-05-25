/***************************************************************************
 * file:        WiseRoute.h
 *
 * author:      Damien Piguet, Jerome Rousselot
 *
 * copyright:   (C) 2007-2009 CSEM SA, Neuchatel, Switzerland.
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
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * ported to Mixim 2.0.1 by Theodoros Kapourniotis
 * last modification: 06/02/11
 **************************************************************************/

#ifndef wiseroute_h
#define wiseroute_h

#include <omnetpp.h>

#include <BaseNetwLayer.h>
#include <BaseMobility.h>
#include <fstream>
#include "WiseRoutePkt_m.h"
#include "MacPkt_m.h"
#include "BaseMacLayer.h"
#include "SimTracer.h"
#include "NetwControlInfo.h"
#include "NetwToMacControlInfo.h"
#include "MacToNetwControlInfo.h"

#include <map>
#include <list>
#include <math.h>

using namespace std;

/**
 * @brief Wiseroute is a simple loop-free routing algorithm that
 * builds a routing tree from a central network point. It is especially
 * useful for wireless sensor networks and convergecast traffic,
 * hence its name (Wireless Sensors Routing).
 * The sink (the device at the center of the network) broadcasts
 * a route building message. Each network node that receives it
 * selects the sink as parent in the routing tree, and rebroadcasts
 * the route building message. This procedure maximizes the probability
 * that all network nodes can join the network, and avoids loops.
 *
 * @ingroup netwLayer
 * @author Jerome Rousselot
 **/
class WiseRoute : public BaseNetwLayer
{
public:
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();

    ~WiseRoute();

protected:
	enum messagesTypes {
	    UNKNOWN=0,
	    DATA,
	    ROUTE_FLOOD,
	    SEND_ROUTE_FLOOD_TIMER
	};

	typedef enum floodTypes {
		NOTAFLOOD,
		FORWARD,
		FORME,
		DUPLICATE
	} floodTypes;


	typedef struct tRouteTableEntry {
		int nextHop;
		double rssi;
	} tRouteTableEntry;

	typedef map<int, tRouteTableEntry> tRouteTable;
	typedef multimap<int, unsigned long> tFloodTable;

	tRouteTable routeTable;
	tFloodTable floodTable;

    /**
     * @brief Length of the NetwPkt header
     * Read from omnetpp.ini
     **/
    int headerLength;



    /** @brief cached variable of my network address */
//    int myNetwAddr;
    int macaddress;

    int sinkAddress;

	bool useSimTracer;

    /** @brief Minimal received RSSI necessary for adding source to routing table. */
    double rssiThreshold;

    /** @brief Interval [seconds] between two route floods. A route flood is a simple flood from
     *         which other nodes can extract routing (next hop) information.
     */
    double routeFloodsInterval;

    /** @brief Flood sequence number */
    unsigned long floodSeqNumber;

    SimTracer *tracer;
    cMessage* routeFloodTimer;

    long nbDataPacketsForwarded;
    long nbDataPacketsReceived;
    long nbDataPacketsSent;
    long nbDuplicatedFloodsReceived;
    long nbFloodsSent;
    long nbPureUnicastSent;
    long nbRouteFloodsSent;
    long nbRouteFloodsReceived;
    long nbUnicastFloodForwarded;
    long nbPureUnicastForwarded;
    long nbGetRouteFailures;
    long nbRoutesRecorded;

    cOutVector receivedRSSI;
    cOutVector routeRSSI;
    cOutVector allReceivedRSSI;
    cOutVector allReceivedBER;
    cOutVector routeBER;
    cOutVector receivedBER;
    cOutVector nextHopSelectionForSink;

    bool trace, stats, debug;

    /**
     * @name Handle Messages
     * @brief Functions to redefine by the programmer
     *
     * These are the functions provided to add own functionality to your
     * modules. These functions are called whenever a self message or a
     * data message from the upper or lower layer arrives respectively.
     *
     **/
    /*@{*/

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage* msg);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage* msg);

    /** @brief Handle self messages */
    virtual void handleSelfMsg(cMessage* msg);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage* msg);

    /** @brief Update routing table.
     *
     * The tuple provided in argument gives the next hop address to the origin.
     * The table is updated only if the RSSI value is above the threshold.
     */
    virtual void updateRouteTable(int origin, int lastHop, double rssi, double ber);

    /** @brief Decapsulate a message */
    cMessage* decapsMsg(WiseRoutePkt *msg);

    /** @brief update flood table. returns detected flood type (general or unicast flood to forward,
     *         duplicate flood to delete, unicast flood to me
     */
    floodTypes updateFloodTable(bool isFlood, int srcAddr, int destAddr, unsigned long seqNum);

    /** @brief find a route to destination address. */
    int getRoute(int destAddr, bool iAmOrigin = false);
};

#endif
