/***************************************************************************
 * file:        Aggregation.h
 *
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2010 CSEM SA, Neuchatel, Switzerland.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * description: this class aggregates the packets received from the application
 * layer and separates packet emissions by a time InterPacketDelay.
 ***************************************************************************/


#ifndef AGGREGATION_H_
#define AGGREGATION_H_

#include <omnetpp.h>
#include <map>
#include <list>

#include "MiXiMDefs.h"
#include "BaseLayer.h"
#include "SimpleAddress.h"

class ApplPkt;

/**
 * @brief this class aggregates the packets received from the application
 * layer and separates packet emissions by a time InterPacketDelay.
 *
 */
class MIXIM_API Aggregation: public BaseLayer {
public:
	Aggregation();
	virtual void initialize(int);
	virtual void finish();
protected:
	    virtual void handleSelfMsg(cMessage* msg);
	    virtual void handleUpperMsg(cMessage *msg);
	    virtual void handleLowerMsg(cMessage *msg);
	    virtual void handleLowerControl(cMessage *msg);
	    virtual void handleUpperControl(cMessage *msg);
private:
	    // this type is used to store, for a network destination, the time
	    // at which a packet was last sent to it, and the packets currently
	    // queued for aggregation.
	    typedef std::pair<simtime_t, std::list<ApplPkt*> > destInfo;

	    // this map associates to each known netwok address
	    // the time at which a packet was last sent to it, and a vector
	    // of packets currently queued for it (waiting aggregation).
	    std::map<LAddress::L3Type, destInfo> destInfos;

	    // This message is used as a timer to perform aggregation
	    cMessage* aggregationTimer;

	    // This is the minimum time between two transmissions to a same network destination
	    simtime_t interPacketDelay;

	    // maximum number of packets to aggregate into a single unit.
	    int nbMaxPacketsPerAggregation;

	    // returns true if we can send now to this destination
	    virtual bool isOkToSendNow(const LAddress::L3Type& dest);

	    // sends aggregated packets to destination now
	    void sendAggregatedPacketNow(const LAddress::L3Type& dest);


	    // counters
	    long nbAggrPktSentDown;
	    long nbAggrPktReceived;
};

#endif /* AGGREGATION_H_ */
