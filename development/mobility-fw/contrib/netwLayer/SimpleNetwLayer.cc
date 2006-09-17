/***************************************************************************
 * file:        SimpleNetwLayer.cc
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
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: network layer: general class for the network layer
 *              subclass to create your own network layer
 ***************************************************************************/


#include "SimpleNetwLayer.h"
#include "NetwControlInfo.h"
#include "MacControlInfo.h"

Define_Module(SimpleNetwLayer);

void SimpleNetwLayer::initialize(int stage)
{
    BasicLayer::initialize(stage);

    if(stage==1){
        headerLength= par("headerLength");
        arp = SimpleArpAccess().get();
        myNetwAddr = this->id();
        EV << " myNetwAddr " << myNetwAddr << endl;
    }
}

/**
 * Decapsulates the packet from the received Network packet 
 **/
cMessage* SimpleNetwLayer::decapsMsg(NetwPkt *msg) 
{
    cMessage *m = msg->decapsulate();
    m->setControlInfo(new NetwControlInfo(msg->getSrcAddr()));
    // delete the netw packet
    delete msg;
    return m;
}


/**
 * Encapsulates the received ApplPkt into a NetwPkt and set all needed
 * header fields.
 **/
NetwPkt* SimpleNetwLayer::encapsMsg(cMessage *msg) {    
    int macAddr;
    int netwAddr;

    EV <<"in encaps...\n";

    NetwPkt *pkt = new NetwPkt(msg->name(), msg->kind());
    pkt->setLength(headerLength);
    
    NetwControlInfo* cInfo = dynamic_cast<NetwControlInfo*>(msg->removeControlInfo());

    if(cInfo == 0){
	EV << "warning: Application layer did not specifiy a destination L3 address\n"
	   << "\tusing broadcast address instead\n";
	netwAddr = L3BROADCAST;
    } else {
	EV <<"CInfo removed, netw addr="<< cInfo->getNetwAddr()<<endl;
        netwAddr = cInfo->getNetwAddr();
	delete cInfo;
    }
    
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

    pkt->setControlInfo(new MacControlInfo(macAddr));
    
    //encapsulate the application packet
    pkt->encapsulate(msg);
    EV <<" pkt encapsulated\n";
    return pkt;
}

/**
 * Redefine this function if you want to process messages from lower
 * layers before they are forwarded to upper layers
 *
 *
 * If you want to forward the message to upper layers please use
 * @ref sendUp which will take care of decapsulation and thelike
 **/
void SimpleNetwLayer::handleLowerMsg(cMessage* msg)
{

    NetwPkt *m = static_cast<NetwPkt *>(msg);
    EV << " handling packet from " << m->getSrcAddr() << endl;
    sendUp(decapsMsg(m));
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to lower layers.
 *
 * For the SimpleNetwLayer we just use the destAddr of the network
 * message as a nextHop
 *
 * To forward the message to lower layers after processing it please
 * use @ref sendDown. It will take care of anything needed
 **/
void SimpleNetwLayer::handleUpperMsg(cMessage* msg)
{
    sendDown(encapsMsg(msg));
}

