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
#include "DummyRoute.h"

#include <limits>
#include <algorithm>
#include <cassert>

#include "DummyRoutePkt_m.h"
#include "NetwControlInfo.h"
#include "ArpInterface.h"
#include "NetwPkt_m.h"

using std::endl;

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
//	LAddress::L2Type nextHopMacAddr;
//	if (cInfo == 0) {
//		EV<<"DummyRoute warning: Application layer did not specifiy a destination L3 address\n"
//	       << "\tusing broadcast address instead\n";
//		nextHopMacAddr = LAddress::L2BROADCAST;
//	} else {
//		nextHopMacAddr = arp->getMacAddr(cInfo->getNetwAddr());
//	}
//	LAddress::setL3ToL2ControlInfo(msg, myNetwAddr, nextHopMacAddr);
	sendDown(encapsMsg(check_and_cast<cPacket*>(msg)));
}

void DummyRoute::finish() {
}

NetwPkt* DummyRoute::encapsMsg(cPacket *appPkt) {
    LAddress::L2Type macAddr;
    LAddress::L3Type netwAddr;

    debugEV <<"in encaps...\n";

    DummyRoutePkt *pkt = new DummyRoutePkt(appPkt->getName(), appPkt->getKind());
    pkt->setBitLength(headerLength);

    cObject* cInfo = appPkt->removeControlInfo();

    if(cInfo == NULL){
	  EV << "warning: Application layer did not specifiy a destination L3 address\n"
	   << "\tusing broadcast address instead\n";
	  netwAddr = LAddress::L3BROADCAST;
    } else {
	  debugEV <<"CInfo removed, netw addr="<< NetwControlInfo::getAddressFromControlInfo( cInfo ) << endl;
	  netwAddr = NetwControlInfo::getAddressFromControlInfo( cInfo );
	  delete cInfo;
    }

    pkt->setNetworkID(networkID);
    pkt->setSrcAddr(myNetwAddr);
    pkt->setDestAddr(netwAddr);
    debugEV << " netw "<< myNetwAddr << " sending packet" <<endl;
    if(LAddress::isL3Broadcast(netwAddr)) {
        debugEV << "sendDown: nHop=L3BROADCAST -> message has to be broadcasted"
           << " -> set destMac=L2BROADCAST\n";
        macAddr = LAddress::L2BROADCAST;
    }
    else{
        debugEV <<"sendDown: get the MAC address\n";
        macAddr = arp->getMacAddr(netwAddr);
    }

    setDownControlInfo(pkt, macAddr);

    //encapsulate the application packet
    pkt->encapsulate(appPkt);
    debugEV <<" pkt encapsulated\n";
    return pkt;
}

cMessage* DummyRoute::decapsMsg(NetwPkt *msg) {
	cMessage *m = msg->decapsulate();
	setUpControlInfo(m, msg->getSrcAddr());
		// delete the netw packet
	delete msg;
	return m;
}
