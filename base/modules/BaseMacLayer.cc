/***************************************************************************
 * file:        BaseMacLayer.cc
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
 * description: basic MAC layer class
 *              subclass to create your own MAC layer
 **************************************************************************/


#include "BaseMacLayer.h"
#include "MacControlInfo.h"
#include "SimpleAddress.h"
#include "NicControlType.h"

#include <cassert>

Define_Module(BaseMacLayer);

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BaseLayer.
 *
 **/
void BaseMacLayer::initialize(int stage)
{
    BaseLayer::initialize(stage);
    if(stage==0) {
    	// get handle to phy layer
        phy = FindModule<MacToPhyInterface*>::findSubModule(getParentModule());

        headerLength= par("headerLength");

        // The nic id is used as MAC address
        myMacAddr = getParentModule()->getId();

        hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;
    }
}

/**
 * Decapsulates the network packet from the received MacPkt
 **/
cPacket* BaseMacLayer::decapsMsg(MacPkt* msg)
{
    cPacket *m = msg->decapsulate();
    m->setControlInfo(new MacControlInfo(msg->getSrcAddr()));
    // delete the macPkt
    delete msg;
    coreEV << " message decapsulated " << endl;
    return m;
}


/**
 * Encapsulates the received NetwPkt into a MacPkt and set all needed
 * header fields.
 **/
MacPkt* BaseMacLayer::encapsMsg(cPacket *netwPkt)
{
    MacPkt *pkt = new MacPkt(netwPkt->getName(), netwPkt->getKind());
    pkt->setBitLength(headerLength);

    // copy dest address from the Control Info attached to the network
    // message by the network layer
    MacControlInfo* cInfo = static_cast<MacControlInfo*>(netwPkt->removeControlInfo());

    coreEV <<"CInfo removed, mac addr="<< cInfo->getNextHopMac()<<endl;
    pkt->setDestAddr(cInfo->getNextHopMac());

    //delete the control info
    delete cInfo;

    //set the src address to own mac address (nic module getId())
    pkt->setSrcAddr(myMacAddr);

    //encapsulate the network packet
    pkt->encapsulate(netwPkt);
    coreEV <<"pkt encapsulated\n";

    return pkt;
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to lower layers.
 *
 * To forward the message to lower layers after processing it please
 * use @ref sendDown. It will take care of anything needed
 **/
void BaseMacLayer::handleUpperMsg(cMessage *mac)
{
	assert(dynamic_cast<cPacket*>(mac));
    sendDown(encapsMsg(static_cast<cPacket*>(mac)));
}

/**
 * This basic implementation just forwards all message that are
 * broadcast (destAddr = L2BROADCAST) or destined for this node
 * (destAddr = nic module getId()) to the network layer
 *
 * @sa sendUp
 **/

void BaseMacLayer::handleLowerMsg(cMessage *msg)
{
    MacPkt *mac = static_cast<MacPkt *>(msg);
    int dest = mac->getDestAddr();
    int src = mac->getSrcAddr();

    //only foward to upper layer if message is for me or broadcast
    if((dest == myMacAddr) || (dest == L2BROADCAST)) {
		coreEV << "message with mac addr " << src
			   << " for me (dest=" << dest
			   << ") -> forward packet to upper layer\n";
		sendUp(decapsMsg(mac));
    }
    else{
		coreEV << "message with mac addr " << src
			   << " not for me (dest=" << dest
			   << ") -> delete (my MAC="<<myMacAddr<<")\n";
		delete mac;
    }
}

void BaseMacLayer::handleLowerControl(cMessage* msg)
{
	switch (msg->getKind())
	{
		case NicControlType::TX_END:
			sendControlUp(msg);
			break;
		default:
			coreEV << "BaseMacLayer does not handle control messages of this type (name was "<<msg->getName()<<")\n";
			delete msg;
			break;
	}
}

Signal* BaseMacLayer::createSignal(simtime_t start, simtime_t length, double power, double bitrate)
{
	simtime_t end = start + length;
	//create signal with start at current simtime and passed length
	Signal* s = new Signal(start, length);

	//create and set tx power mapping
	Mapping* txPowerMapping = createConstantMapping(start, end, power);
	s->setTransmissionPower(txPowerMapping);

	//create and set bitrate mapping
	Mapping* bitrateMapping = createConstantMapping(start, end, bitrate);
	s->setBitrate(bitrateMapping);

	return s;
}

Mapping* BaseMacLayer::createConstantMapping(simtime_t start, simtime_t end, double value)
{
	//create mapping over time
	Mapping* m = MappingUtils::createMapping(0.0, DimensionSet(Dimension::time), Mapping::LINEAR);

	//set position Argument
	Argument startPos(start);

	//set mapping at position
	m->setValue(startPos, value);

	//set position Argument
	Argument endPos(end);

	//set mapping at position
	m->setValue(endPos, value);

	return m;
}





