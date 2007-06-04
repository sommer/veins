/* -*- mode:c++ -*- ********************************************************
 * file:        BaseAggLayer.cc
 *
 * author:      Tom Parker
 *
 * copyright:   (C) 2006 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     wsn-specific modules
 * description: aggregation layer: basic core. subclass to build your own
 *              aggregation protocol
 ***************************************************************************/


#include "BaseAggLayer.h"
#include "NetwControlInfo.h"
#include <assert.h>

#include <SinkAddress.h>

Define_Module(BaseAggLayer);

void BaseAggLayer::initialize(int stage)
{
	BaseLayer::initialize(stage);

	if (stage == 1)
	{
		headerLength = 0;		//par("headerLength");
		arp = BaseArpAccess().get();
		isSink = getNode()->par("isSink").boolValue();
		//myAggAddr = this->id();
		//EV << " myAggAddr " << myAggAddr << endl;
	}
}

/**
 * Redefine this function if you want to process messages from lower
 * layers before they are forwarded to upper layers
 *
 *
 * If you want to forward the message to upper layers please use
 * @ref sendUp which will take care of decapsulation and thelike
 **/
void BaseAggLayer::handleLowerMsg(cMessage * msg)
{
	AggPkt *m = check_and_cast < AggPkt * >(msg);
	EV << " handling packet from " << m->getSrcAddr() << endl;
	if (isSink)
		sendUp(m);
	else
	{
		delete m->removeControlInfo();
		m->setControlInfo(new NetwControlInfo(SINK_ADDRESS));
		sendDown(m);
	}
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to lower layers.
 *
 * For the BaseAggLayer we just use the destAddr of the Aggork
 * message as a nextHop
 *
 * To forward the message to lower layers after processing it please
 * use @ref sendDown. It will take care of anything needed
 **/
void BaseAggLayer::handleUpperMsg(cMessage * msg)
{
	AggPkt *m = check_and_cast < AggPkt * >(msg);
	m->setControlInfo(new NetwControlInfo(SINK_ADDRESS));
	sendDown(m);
}

