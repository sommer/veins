/* -*- mode:c++ -*- ********************************************************
 * file:        CollisionsPhy.cc
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
 * description: propagation layer - collisions/start msg capable
 ***************************************************************************/


#include "CollisionsPhy.h"
#include <assert.h>

Define_Module(CollisionsPhy);

void CollisionsPhy::initialize(int stage)
{
	BasePhyLayer::initialize(stage);
	if (stage == 0)
		bitrate = par("bitrate");
}

void CollisionsPhy::handleLowerMsg(cMessage *msg)
{
	StartMessage *sm = dynamic_cast<StartMessage*>(msg);
	if (sm!=NULL)
	{
		handleLowerMsgStart(sm);
		return;
	}
	CorruptMessage *c = dynamic_cast<CorruptMessage*>(msg);
	if (c!=NULL)
	{
		handleCorruptMessage(c);
		return;
	}
	// otherwise, should be an AirFrame
	AirFrame *af = dynamic_cast<AirFrame*>(msg);
	assert(af!=NULL);
	handleLowerMsgEnd(af);
}

/**
 * Redefine this function if you want to process messages from the
 * channel before they are forwarded to upper layers
 *
 * This function is called right after a message is received,
 * i.e. right before it is buffered for 'transmission time'.
 *
 * Here you should decide whether the message is "really" received or
 * whether it's receive power is so low that it is just treated as
 * noise.
 *
 **/
void CollisionsPhy::handleLowerMsgStart(StartMessage *msg)
{
    EV <<"in handleLowerMsgStart"<<endl;
	delete msg;
}

/**
 * Redefine this function if you want to process messages from the
 * channel before they are forwarded to upper layers
 *
 * Do not forget to send the message to the upper layer with sendUp()
 */
void CollisionsPhy::handleLowerMsgEnd(AirFrame *msg)
{
    EV << "in handleLowerMsgEnd\n";
    sendUp( decapsMsg(msg) );
}

void CollisionsPhy::handleCorruptMessage(CorruptMessage* c)
{
	EV << "saw a corrupt message, dropping\n";
	delete c;
}

void CollisionsPhy::handleUpperMsg(cMessage *msg)
{
    AirFrame *frame = encapsMsg(msg);
	frame->setDuration(calcDuration(frame));
    setTimer(0,frame->getDuration());
    sendDown(frame);
}
