/* -*- mode:c++ -*- ********************************************************
 * file:        Flood.cc
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 *
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: a simple flooding protocol
 *              the user can decide whether to use plain flooding or not
 **************************************************************************/

#include "Flood.h"

#include <cassert>

#include "NetwPkt_m.h"
#include "NetwControlInfo.h"

using std::endl;

Define_Module(Flood);

/**
 * Reads all parameters from the ini file. If a parameter is not
 * specified in the ini file a default value will be set.
 **/
void Flood::initialize(int stage) {
	BaseNetwLayer::initialize(stage);

	if (stage == 0) {
		//initialize seqence number to 0
		seqNum = 0;
		nbDataPacketsReceived = 0;
		nbDataPacketsSent = 0;
		nbDataPacketsForwarded = 0;
		nbHops = 0;
		hasPar("defaultTtl") ? defaultTtl = par("defaultTtl") : defaultTtl = 5;
		hasPar("plainFlooding") ? plainFlooding = par("plainFlooding")
				: plainFlooding = true;

		EV<< "defaultTtl = " << defaultTtl
		<< " plainFlooding = " << plainFlooding << endl;

		if(plainFlooding) {
			//these parameters are only needed for plain flooding
			hasPar("bcMaxEntries") ? bcMaxEntries = par("bcMaxEntries") : bcMaxEntries = 30;

			hasPar("bcDelTime") ? bcDelTime = par("bcDelTime") : bcDelTime = 3.0;
			EV <<"bcMaxEntries = "<<bcMaxEntries
			<<" bcDelTime = "<<bcDelTime<<endl;
		}
	}
}

void Flood::finish() {
	if (plainFlooding) {
		bcMsgs.clear();
	}
	recordScalar("nbDataPacketsReceived", nbDataPacketsReceived);
	recordScalar("nbDataPacketsSent", nbDataPacketsSent);
	recordScalar("nbDataPacketsForwarded", nbDataPacketsForwarded);
	if (nbDataPacketsReceived > 0) {
	  recordScalar("meanNbHops", (double) nbHops / (double) nbDataPacketsReceived);
	} else {
		recordScalar("meanNbHops", 0);
	}
}

/**
 * All messages have to get a sequence number and the ttl filed has to
 * be specified. Afterwards the messages can be handed to the mac
 * layer. The mac address is set to -1 (broadcast address) because the
 * message is flooded (i.e. has to be send to all neighbors)
 *
 * In the case of plain flooding the message sequence number and
 * source address has also be stored in the bcMsgs list, so that this
 * message will not be rebroadcasted, if a copy will be flooded back
 * from the neigbouring nodes.
 *
 * If the maximum number of entries is reached the first (oldest) entry
 * is deleted.
 **/
void Flood::handleUpperMsg(cMessage* m) {

	assert(dynamic_cast<cPacket*> (m));
	NetwPkt *msg = encapsMsg(static_cast<cPacket*> (m));

	msg->setSeqNum(seqNum);
	seqNum++;
	msg->setTtl(defaultTtl);

	if (plainFlooding) {
		if (bcMsgs.size() >= bcMaxEntries) {
			cBroadcastList::iterator it;

			//serach the broadcast list of outdated entries and delete them
			for (it = bcMsgs.begin(); it != bcMsgs.end(); ++it) {
				if (it->delTime < simTime()) {
					bcMsgs.erase(it);
					it--;
					break;
				}
			}
			//delete oldest entry if max size is reached
			if (bcMsgs.size() >= bcMaxEntries) {
				EV<<"bcMsgs is full, delete oldest entry"<<endl;
				bcMsgs.pop_front();
			}
		}
		bcMsgs.push_back(Bcast(msg->getSeqNum(), msg->getSrcAddr(), simTime() +bcDelTime));
    }
				//there is no routing so all messages are broadacst for the mac layer

				sendDown(msg);
				nbDataPacketsSent++;
			}

			/**
			 * Messages from the mac layer will be forwarded to the application
			 * only if the are broadcast or destined for this node.
			 *
			 * If the arrived message is a broadcast message it is also reflooded
			 * only if the tll field is bigger than one. Before the message is
			 * handed back to the mac layer the ttl field is reduced by one to
			 * account for this hop.
			 *
			 * In the case of plain flooding the message will only be processed if
			 * there is no corresponding entry in the bcMsgs list (@ref
			 * notBroadcasted). Otherwise the message will be deleted.
			 **/
void Flood::handleLowerMsg(cMessage* m) {
	NetwPkt *msg = static_cast<NetwPkt *> (m);

	//msg not broadcastes yet
	if (notBroadcasted(msg)) {
		//msg is for me
		if (msg->getDestAddr() == myNetwAddr) {
			EV<<" data msg for me! send to Upper"<<endl;
			nbHops = nbHops + (defaultTtl + 1 - msg->getTtl());
			sendUp( decapsMsg(msg) );
			nbDataPacketsReceived++;
		}
		//broadcast message
		else if( LAddress::isL3Broadcast(msg->getDestAddr()) ) {
			//check ttl and rebroadcast
			if( msg->getTtl() > 1 ) {
				NetwPkt *dMsg;
				EV <<" data msg BROADCAST! ttl = "<<msg->getTtl()
				<<" > 1 -> rebroadcast msg & send to upper\n";
				msg->setTtl( msg->getTtl()-1 );
				dMsg = static_cast<NetwPkt*>(msg->dup());
				setDownControlInfo(dMsg, LAddress::L2BROADCAST);
				sendDown(dMsg);
				nbDataPacketsForwarded++;
			}
			else
			EV <<" max hops reached (ttl = "<<msg->getTtl()<<") -> only send to upper\n";

			// message has to be forwarded to upper layer
			nbHops = nbHops + (defaultTtl + 1 - msg->getTtl());
			sendUp( decapsMsg(msg) );
			nbDataPacketsReceived++;
		}
		//not for me -> rebroadcast
		else {
			//check ttl and rebroadcast
			if( msg->getTtl() > 1 ) {
				EV <<" data msg not for me! ttl = "<<msg->getTtl()
				<<" > 1 -> forward\n";
				msg->setTtl( msg->getTtl()-1 );
				// needs to set the next hop address again to broadcast
				cObject *const pCtrlInfo = msg->removeControlInfo();
				if (pCtrlInfo != NULL)
					delete pCtrlInfo;
				setDownControlInfo(msg, LAddress::L2BROADCAST);
				sendDown( msg );
				nbDataPacketsForwarded++;
			}
			else {
				//max hops reached -> delete
				EV <<" max hops reached (ttl = "<<msg->getTtl()<<") -> delete msg\n";
				delete msg;
			}
		}
	}
	else {
		EV <<" data msg already BROADCASTed! delete msg\n";
		delete msg;
	}
}

/**
 * The bcMsgs list is searched for the arrived message. If the message
 * is in the list, it was already broadcasted and the function returns
 * false.
 *
 * Concurrently all outdated (older than bcDelTime) are deleted. If
 * the list is full and a new message has to be entered, the oldest
 * entry is deleted.
 **/
bool Flood::notBroadcasted(NetwPkt* msg) {
	if (!plainFlooding)
		return true;

	cBroadcastList::iterator it;

	//serach the broadcast list of outdated entries and delete them
	for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
		if (it->delTime < simTime()) {
			bcMsgs.erase(it);
			it--;
		}
		//message was already broadcasted
		if ((it->srcAddr == msg->getSrcAddr()) && (it->seqNum
				== msg->getSeqNum())) {
			// update entry
			it->delTime = simTime() + bcDelTime;
			return false;
		}
	}

	//delete oldest entry if max size is reached
	if (bcMsgs.size() >= bcMaxEntries) {
		EV<<"bcMsgs is full, delete oldest entry\n";
		bcMsgs.pop_front();
	}

	bcMsgs.push_back(Bcast(msg->getSeqNum(), msg->getSrcAddr(), simTime() +bcDelTime));
    return true;
}

NetwPkt* Flood::encapsMsg(cPacket *appPkt) {
    LAddress::L2Type macAddr;
    LAddress::L3Type netwAddr;

    EV<<"in encaps...\n";

    NetwPkt *pkt = new NetwPkt(appPkt->getName(), appPkt->getKind());
    pkt->setBitLength(headerLength);

    cObject* cInfo = appPkt->removeControlInfo();

    if(cInfo == NULL){
	EV << "warning: Application layer did not specifiy a destination L3 address\n"
	   << "\tusing broadcast address instead\n";
	netwAddr = LAddress::L3BROADCAST;
    } else {
	EV <<"CInfo removed, netw addr="<< NetwControlInfo::getAddressFromControlInfo( cInfo ) <<endl;
        netwAddr = NetwControlInfo::getAddressFromControlInfo( cInfo );
	delete cInfo;
    }

    pkt->setSrcAddr(myNetwAddr);
    pkt->setDestAddr(netwAddr);
    EV << " netw "<< myNetwAddr << " sending packet" <<endl;

    EV << "sendDown: nHop=L3BROADCAST -> message has to be broadcasted"
       << " -> set destMac=L2BROADCAST" << endl;
    macAddr = LAddress::L2BROADCAST;

    setDownControlInfo(pkt, macAddr);

    //encapsulate the application packet
    pkt->encapsulate(appPkt);
    EV <<" pkt encapsulated\n";
    return pkt;
}
