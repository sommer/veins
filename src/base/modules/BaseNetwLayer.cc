/***************************************************************************
 * file:        BaseNetwLayer.cc
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


#include "BaseNetwLayer.h"

#include <cassert>

#include "NetwControlInfo.h"
#include "BaseMacLayer.h"
#include "AddressingInterface.h"
#include "SimpleAddress.h"
#include "FindModule.h"
#include "NetwPkt_m.h"
#include "ArpInterface.h"
#include "NetwToMacControlInfo.h"

Define_Module(BaseNetwLayer);

void BaseNetwLayer::initialize(int stage)
{
    BaseLayer::initialize(stage);

    if(stage==0){
    	coreDebug = par("coreDebug").boolValue();
        headerLength= par("headerLength");
        arp = FindModule<ArpInterface*>::findSubModule(findHost());
    }
    else if(stage == 1) {
    	// see if there is an addressing module available
    	// otherwise use module id as network address
        AddressingInterface* addrScheme = FindModule<AddressingInterface*>
													::findSubModule(findHost());
        if(addrScheme) {
        	myNetwAddr = addrScheme->myNetwAddr(this);
        } else {
        	myNetwAddr = LAddress::L3Type( getId() );
        }
        coreEV << " myNetwAddr " << myNetwAddr << std::endl;
    }
}

/**
 * Decapsulates the packet from the received Network packet
 **/
cMessage* BaseNetwLayer::decapsMsg(NetwPkt *msg)
{
    cMessage *m = msg->decapsulate();
    setUpControlInfo(m, msg->getSrcAddr());
    // delete the netw packet
    delete msg;
    return m;
}


/**
 * Encapsulates the received ApplPkt into a NetwPkt and set all needed
 * header fields.
 **/
NetwPkt* BaseNetwLayer::encapsMsg(cPacket *appPkt) {
    LAddress::L2Type macAddr;
    LAddress::L3Type netwAddr;

    coreEV <<"in encaps...\n";

    NetwPkt *pkt = new NetwPkt(appPkt->getName(), appPkt->getKind());
    pkt->setBitLength(headerLength);

    cObject* cInfo = appPkt->removeControlInfo();

    if(cInfo == NULL){
	EV << "warning: Application layer did not specifiy a destination L3 address\n"
	   << "\tusing broadcast address instead\n";
	netwAddr = LAddress::L3BROADCAST;
    } else {
	coreEV <<"CInfo removed, netw addr="<< NetwControlInfo::getAddressFromControlInfo( cInfo ) << std::endl;
        netwAddr = NetwControlInfo::getAddressFromControlInfo( cInfo );
	delete cInfo;
    }

    pkt->setSrcAddr(myNetwAddr);
    pkt->setDestAddr(netwAddr);
    coreEV << " netw "<< myNetwAddr << " sending packet" <<std::endl;
    if(LAddress::isL3Broadcast( netwAddr )) {
        coreEV << "sendDown: nHop=L3BROADCAST -> message has to be broadcasted"
           << " -> set destMac=L2BROADCAST\n";
        macAddr = LAddress::L2BROADCAST;
    }
    else{
        coreEV <<"sendDown: get the MAC address\n";
        macAddr = arp->getMacAddr(netwAddr);
    }

    setDownControlInfo(pkt, macAddr);

    //encapsulate the application packet
    pkt->encapsulate(appPkt);
    coreEV <<" pkt encapsulated\n";
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
void BaseNetwLayer::handleLowerMsg(cMessage* msg)
{
    NetwPkt *m = static_cast<NetwPkt *>(msg);
    coreEV << " handling packet from " << m->getSrcAddr() << std::endl;
    sendUp(decapsMsg(m));
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to lower layers.
 *
 * For the BaseNetwLayer we just use the destAddr of the network
 * message as a nextHop
 *
 * To forward the message to lower layers after processing it please
 * use @ref sendDown. It will take care of anything needed
 **/
void BaseNetwLayer::handleUpperMsg(cMessage* msg)
{
	assert(dynamic_cast<cPacket*>(msg));
    sendDown(encapsMsg(static_cast<cPacket*>(msg)));
}

/**
 * Redefine this function if you want to process control messages
 * from lower layers.
 *
 * This function currently handles one messagetype: TRANSMISSION_OVER.
 * If such a message is received in the network layer it is deleted.
 * This is done as this type of messages is passed on by the BaseMacLayer.
 *
 * It may be used by network protocols to determine when the lower layers
 * are finished sending a message.
 **/
void BaseNetwLayer::handleLowerControl(cMessage* msg)
{
	switch (msg->getKind())
	{
	case BaseMacLayer::TX_OVER:
		delete msg;
		break;
	default:
		EV << "BaseNetwLayer does not handle control messages called "
		   << msg->getName() << std::endl;
		delete msg;
		break;
	}
}

/**
 * Attaches a "control info" structure (object) to the down message pMsg.
 */
cObject *const BaseNetwLayer::setDownControlInfo(cMessage *const pMsg, const LAddress::L2Type& pDestAddr)
{
	return NetwToMacControlInfo::setControlInfo(pMsg, pDestAddr);
}

/**
 * Attaches a "control info" structure (object) to the up message pMsg.
 */
cObject *const BaseNetwLayer::setUpControlInfo(cMessage *const pMsg, const LAddress::L3Type& pSrcAddr)
{
	return NetwControlInfo::setControlInfo(pMsg, pSrcAddr);
}
