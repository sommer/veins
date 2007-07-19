/***************************************************************************
 * file:        BaseLocalization.cc
 *
 * author:      Peterpaul Klein Haneveld
 *
 * copyright:   (C) 2007 Parallel and Distributed Systems Group (PDS) at
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
 * part of:     mixim framework
 * description: localization layer: general class for the network layer
 *              subclass to create your own localization layer
 ***************************************************************************/


#include "BaseLocalization.h"
#include "BaseUtility.h"
#include "NetwControlInfo.h"
#include "FindModule.h"

Define_Module(BaseLocalization);

void BaseLocalization::initialize(int stage)
{
	BaseLayer::initialize(stage);

	if (stage == 1) {
		headerLength = par("headerLength");
	}
}

Coord BaseLocalization::getPosition()
{
	BaseUtility *util =
	    FindModule < BaseUtility * >::findSubModule(findHost());
	return *util->getPos();
}

/**
 * Decapsulates the packet from the received Network packet 
 **/
cMessage *BaseLocalization::decapsMsg(LocPkt * msg)
{
	cMessage *m = msg->decapsulate();
//      Coord position = getPosition();

	EV << " pkt decapsulated\n";

	// pass some information up?
//      m->setControlInfo(new LocControlInfo(position));

	// delete the localization packet
	delete msg;
	return m;
}


/**
 * Encapsulates the received ApplPkt into a LocPkt and set all needed
 * header fields.
 **/
LocPkt *BaseLocalization::encapsMsg(cMessage * msg)
{
	LocPkt *pkt = new LocPkt(msg->name(), msg->kind());
	pkt->setLength(headerLength);

	NetwControlInfo *cInfo =
	    dynamic_cast < NetwControlInfo * >(msg->removeControlInfo());

	if (cInfo != NULL) {
		pkt->setControlInfo(cInfo);
	}
	//encapsulate the application packet
	pkt->encapsulate(msg);

	EV << " pkt encapsulated\n";
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
void BaseLocalization::handleLowerMsg(cMessage * msg)
{

	LocPkt *m = static_cast < LocPkt * >(msg);
	EV << " handling packet from " << m->getSrcAddr() << endl;
	sendUp(decapsMsg(m));
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to lower layers.
 *
 * For the BaseLocalization we just use the destAddr of the network
 * message as a nextHop
 *
 * To forward the message to lower layers after processing it please
 * use @ref sendDown. It will take care of anything needed
 **/
void BaseLocalization::handleUpperMsg(cMessage * msg)
{
	sendDown(encapsMsg(msg));
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
void BaseLocalization::handleLowerControl(cMessage * msg)
{
	switch (msg->kind()) {
	default:
		opp_warning
		    ("BaseLocalization does not handle control messages called %s",
		     msg->name());
		delete msg;
	}
}
