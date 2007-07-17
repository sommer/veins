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

Define_Module_Like(CollisionsPhy, BasePhyLayer);

void CollisionsPhy::initialize(int stage)
{
	BasePhyLayer::initialize(stage);
	if (stage == 0)
	{
		messages = 0;
		colliding = false;
	}
}

void CollisionsPhy::handleUpperMsg(cMessage *msg)
{
	increment();
 	EV << "in handleUpperMsg\n";
	BasePhyLayer::handleUpperMsg(msg);
}

void CollisionsPhy::handleLowerMsgStart(cMessage *msg)
{
	increment();
 	EV << "in handleLowerMsgStart\n";
	BasePhyLayer::handleLowerMsgStart(msg);
}

void CollisionsPhy::handleLowerMsgEnd(cMessage *msg)
{
	decrement(); 	
 	EV << "in handleLowerMsgEnd\n";

	if (!colliding)
    	sendUp( decapsMsg(static_cast<AirFrame *>(msg)) );
	else
		EV << "Collision! Dropped message\n";
}

void CollisionsPhy::handleTransmissionOver()
{
	decrement(); 	
 	EV << "in handleTransmissionOver\n";
	BasePhyLayer::handleTransmissionOver();
}

void CollisionsPhy::increment()
{
	messages++;
	if (messages>1)
		colliding = true;
}

void CollisionsPhy::decrement()
{
	assert(messages>0);
	messages--;
	if (messages == 0)
		colliding = false;
}

