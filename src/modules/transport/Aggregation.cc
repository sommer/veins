#include "Aggregation.h"

#include <iostream>
#include <cassert>

#include "ApplPkt_m.h"
#include "AggrPkt.h"

using std::map;

Define_Module(Aggregation);

Aggregation::Aggregation() :
		aggregationTimer(NULL)
{}

void Aggregation::initialize(int stage) {
    BaseLayer::initialize(stage);
	if(stage == 0) {
		interPacketDelay = par("interPacketDelay").doubleValue();
		if(interPacketDelay > 0) {
		  nbMaxPacketsPerAggregation = par("nbMaxPacketsPerAggregation");
		  assert(nbMaxPacketsPerAggregation > 0);
		  aggregationTimer = new cMessage("AggregationTimer");
		  nbAggrPktSentDown = 0;
		  nbAggrPktReceived = 0;
		} else {
		  interPacketDelay = 0;
		}
	}
}

bool Aggregation::isOkToSendNow(const LAddress::L3Type& dest) {
	bool isOkToSendNow = false;
	map<LAddress::L3Type, destInfo>::iterator iter = destInfos.find(dest);
	if(iter == destInfos.end()) {
		// we can send directly if we meet this node for the first time
		isOkToSendNow = true;
	} else if(destInfos[dest].first + interPacketDelay < simTime()) {
		// we can send directly if the interPacketDelay time has expired since last transmission
		isOkToSendNow = true;
		assert(destInfos[dest].second.size() == 0); // otherwise the aggregation timer should have fired
	}
	return isOkToSendNow;
}

void Aggregation::handleUpperMsg(cMessage* msg) {
	ApplPkt* pkt = check_and_cast<ApplPkt*> (msg);
	if (interPacketDelay == 0) {
		sendDown(msg);
	} else {
		const LAddress::L3Type& dest = pkt->getDestAddr();
		if (!isOkToSendNow(dest)) {
			// store packet
			destInfos[dest].second.push_back(pkt);
			// reschedule aggregation timer to "earliest destination"
			simtime_t destTxTime = destInfos[dest].first + interPacketDelay;
			if (aggregationTimer->isScheduled()) {
				if (aggregationTimer->getArrivalTime() > destTxTime) {
					cancelEvent( aggregationTimer);
					scheduleAt(destTxTime, aggregationTimer);
				}
			} else {
				scheduleAt(destTxTime, aggregationTimer);
			}
		} else {
			// send now
			destInfos[dest].second.push_back(pkt);
			sendAggregatedPacketNow(dest);
		}
	}
}

void Aggregation::sendAggregatedPacketNow(const LAddress::L3Type& dest) {
  AggrPkt* aggr = new AggrPkt("AggregationPacket", 1);
  aggr->setBitLength(8);
  int nbAggr = 0;
  int pktSize = 0;
  cObject* ctrlInfo = NULL;
  while(nbAggr < nbMaxPacketsPerAggregation && destInfos[dest].second.size() > 0) {
	  pktSize = pktSize + destInfos[dest].second.front()->getByteLength();
	  if(ctrlInfo != NULL) {
		  delete ctrlInfo; // we delete all ctrlInfo except the last, which we attach to our message
	  }
	  ctrlInfo = destInfos[dest].second.front()->getControlInfo();
	  aggr->storePacket(destInfos[dest].second.front());
	  destInfos[dest].second.pop_front();
	  nbAggr = nbAggr + 1;
  }
  aggr->setByteLength(pktSize); // why doesn't this compile ?
  aggr->setControlInfo(ctrlInfo);
  sendDown(aggr);
  destInfos[dest].first = simTime();
  nbAggrPktSentDown++;
}

void Aggregation::handleLowerMsg(cMessage * msg) {
	if(interPacketDelay == 0) {
		sendUp(msg);
	} else {
	  AggrPkt* aggr = check_and_cast<AggrPkt*>(msg);
	  while(!aggr->isEmpty()) {
		ApplPkt* aPacket = aggr->popFrontPacket();
		sendUp(aPacket);
	  }
	  delete aggr;
	  nbAggrPktReceived++;
	}
}

void Aggregation::handleSelfMsg(cMessage* msg) {
	ASSERT(msg == aggregationTimer);
	// loop over all destinations
	// and send their packets if the time has come
	map<LAddress::L3Type, destInfo>::const_iterator iter = destInfos.begin();
	// simultaneously, compute next trigger time for aggregate timer (if required)
	simtime_t nextTxTime = simTime() + 2*interPacketDelay;
	while(iter != destInfos.end()) {
		if(iter->second.first + interPacketDelay <= simTime() && iter->second.second.size() > 0) {
			sendAggregatedPacketNow(iter->first);
		}
		if(iter->second.second.size() > 0 && iter->second.first + interPacketDelay < nextTxTime) {
			nextTxTime = iter->second.first + interPacketDelay;
		}
		iter++;
	}
	if(nextTxTime < simTime() + 2*interPacketDelay) {
	  scheduleAt(nextTxTime, aggregationTimer);
	}
}

void Aggregation::finish() {
	// clean up memory
  cancelAndDelete(aggregationTimer);
  map<LAddress::L3Type, destInfo>::iterator iter = destInfos.begin();
  while(iter != destInfos.end()) {
	  while(iter->second.second.size() > 0) {
		  ApplPkt* pkt = iter->second.second.front();
		  delete pkt;
		  iter->second.second.pop_front();
	  }
	  iter++;
  }
  // save counter values
  recordScalar("nbAggrPktReceived", nbAggrPktReceived);
  recordScalar("nbAggrPktSentDown ", nbAggrPktSentDown);
}

void Aggregation::handleLowerControl(cMessage *msg) {
	sendControlUp(msg);
}

void Aggregation::handleUpperControl(cMessage *msg) {
	sendControlDown(msg);
}
