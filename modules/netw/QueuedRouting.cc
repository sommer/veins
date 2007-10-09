/* -*- mode:c++ -*- ********************************************************
 * file:        QueuedRouting.cc
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
 * part of:     routing modules
 * description: network layer: basic routing with queues
 ***************************************************************************/


#include "QueuedRouting.h"
#include "NetwControlInfo.h"
#include "MacControlInfo.h"

Define_Module(QueuedRouting);

void QueuedRouting::toNetwork(NetwPkt * out)
{
	if (msgBusy)
	{
		msgQueue->push(out);
		EV << "Local link is busy, queuing for future send" << endl;
	}
	else
	{
		msgBusy = true;
		EV << "Pushing over local link" << endl;
		sendDown(out);
	}
}

void QueuedRouting::initialize(int stage)
{
	BaseNetwLayer::initialize(stage);

	if (stage == 1)
	{
		headerLength = par("headerLength");
		arp = BaseArpAccess().get();
		msgQueue = new std::queue < NetwPkt * >();
		msgBusy = false;
	}
}

QueuedRouting::~QueuedRouting()
{
	delete msgQueue;
}

void QueuedRouting::finish()
{
	BaseNetwLayer::finish();
}

NetwPkt *QueuedRouting::buildPkt(int kind, int netwAddr, const char *name)
{
	int macAddr;
	NetwPkt *pkt = new NetwPkt(name, kind);
	pkt->setLength(headerLength);
	pkt->setSrcAddr(myNetwAddr);
	pkt->setDestAddr(netwAddr);
	EV << " netw " << myNetwAddr << " sending packet" << endl;
	if (netwAddr == L3BROADCAST)
	{
		EV << "toNetwork: nHop=L3BROADCAST -> message has to be broadcasted" << " -> set destMac=L2BROADCAST\n";
		macAddr = L2BROADCAST;
	}
	else
	{
		EV << "toNetwork: get the MAC address\n";
		macAddr = arp->getMacAddr(netwAddr);
	}

	pkt->setControlInfo(new MacControlInfo(macAddr));

	return pkt;
}

NetwPkt *QueuedRouting::encapsMsg(cMessage * msg)
{
	EV << "in encaps...\n";
	int netwAddr;

	NetwControlInfo *cInfo = dynamic_cast < NetwControlInfo * >(msg->removeControlInfo());

	if (cInfo == NULL)
	{
		error("Application layer did not specify a destination L3 address");
		netwAddr = L3BROADCAST;
	}
	else if (cInfo->getNetwAddr() < 0)
	{
		EV << "cInfo removed, but netwAddr is negative("<<cInfo->getNetwAddr()<<") and therefore is user-defined, so send to broadcast"<<endl;
		netwAddr = L3BROADCAST;
		delete cInfo;
	}
	else
	{
		EV << "CInfo removed, netw addr=" << cInfo->getNetwAddr() << endl;
		netwAddr = cInfo->getNetwAddr();
		delete cInfo;
	}

	NetwPkt *pkt = buildPkt(upperKind(), netwAddr, msg->name());

	//encapsulate the application packet
	pkt->encapsulate(msg);
	EV << " pkt encapsulated\n";
	return pkt;
}	   

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to lower layers.
 *
 * To forward the message to lower layers after processing it please
 * use @ref toNetwork. It will take care of anything needed
 **/

void QueuedRouting::handleUpperMsg(cMessage * msg)
{
	EV << "Sending upper layer packet to network" << endl;
	toNetwork(encapsMsg(msg));
}


void QueuedRouting::handleLowerControl(cMessage * msg)
{
	switch (msg->kind())
	{
		case NicControlType::TRANSMISSION_OVER:
			EV << "Transmission complete" << endl;
			msgBusy = false;
			sendQueued();
			delete msg;
			break;
		default:
			EV << "QueuedRouting does not handle control messages of this type (name was " << msg->name() << " kind was " << msg->kind() << ")" << endl;
			delete msg;
			break;
	}
}

void QueuedRouting::sendQueued()
{
	if (!msgQueue->empty())
	{
		EV << "Sending queued msg" << endl;
		NetwPkt *send = msgQueue->front();
		msgQueue->pop();		// trash message
		toNetwork(send);
	}
}
