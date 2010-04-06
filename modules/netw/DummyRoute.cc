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
#include "DummyRoutePkt_m.h"
#include <NetwToMacControlInfo.h>
#include <cassert>

Define_Module(DummyRoute);

void DummyRoute::initialize(int stage) {
	BaseNetwLayer::initialize(stage);
	if (stage == 0) {
		trace = par("trace");
		networkID = par("networkID");
	}
}


void DummyRoute::handleLowerMsg(cMessage* msg) {
	DummyRoutePkt* pkt = check_and_cast<DummyRoutePkt*>(msg);
	if(pkt->getNetworkID()==networkID) {
		sendUp(decapsMsg(pkt));
	} else {
		delete pkt;
	}
}

void DummyRoute::handleLowerControl(cMessage *msg) {
	sendControlUp(msg);
}

void DummyRoute::handleUpperMsg(cMessage* msg) {
//	NetwControlInfo* cInfo =
//			dynamic_cast<NetwControlInfo*> (msg->removeControlInfo());
//	int nextHopMacAddr;
//	if (cInfo == 0) {
//		EV<<"DummyRoute warning: Application layer did not specifiy a destination L3 address\n"
//	       << "\tusing broadcast address instead\n";
//		nextHopMacAddr = 0;
//	} else {
//		nextHopMacAddr = cInfo->getNetwAddr();
//	}
//	nextHopMacAddr = cInfo->getNetwAddr();
//	msg->setControlInfo(new NetwToMacControlInfo(nextHopMacAddr));
	sendDown(encapsMsg(check_and_cast<cPacket*>(msg)));
}

void DummyRoute::finish() {
}

NetwPkt* DummyRoute::encapsMsg(cPacket *appPkt) {
    int macAddr;
    int netwAddr;

    EV <<"in encaps...\n";

    DummyRoutePkt *pkt = new DummyRoutePkt(appPkt->getName(), appPkt->getKind());
    pkt->setBitLength(headerLength);

    NetwControlInfo* cInfo = dynamic_cast<NetwControlInfo*>(appPkt->removeControlInfo());

    if(cInfo == 0){
	  EV << "warning: Application layer did not specifiy a destination L3 address\n"
	   << "\tusing broadcast address instead\n";
	  netwAddr = L3BROADCAST;
    } else {
	  EV <<"CInfo removed, netw addr="<< cInfo->getNetwAddr()<<endl;
        netwAddr = cInfo->getNetwAddr();
	  delete cInfo;
    }

    pkt->setNetworkID(networkID);
    pkt->setSrcAddr(myNetwAddr);
    pkt->setDestAddr(netwAddr);
    EV << " netw "<< myNetwAddr << " sending packet" <<endl;
    if(netwAddr == L3BROADCAST) {
        EV << "sendDown: nHop=L3BROADCAST -> message has to be broadcasted"
           << " -> set destMac=L2BROADCAST\n";
        macAddr = L2BROADCAST;
    }
    else{
        EV <<"sendDown: get the MAC address\n";
        macAddr = arp->getMacAddr(netwAddr);
    }

    pkt->setControlInfo(new NetwToMacControlInfo(macAddr));

    //encapsulate the application packet
    pkt->encapsulate(appPkt);
    EV <<" pkt encapsulated\n";
    return pkt;
}

cMessage* DummyRoute::decapsMsg(NetwPkt *msg) {
	cMessage *m = msg->decapsulate();
	m->setControlInfo(new NetwControlInfo(msg->getSrcAddr()));
		// delete the netw packet
	delete msg;
	return m;
}
