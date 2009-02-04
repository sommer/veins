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
 *
 * description: phy layer - collisions/start msg capable
 ***************************************************************************/


#include "CollisionsPhy.h"
#include <assert.h>

//Define_Module_Like(CollisionsPhy, BasePhyLayer);

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
	BasePhyLayer::handleUpperMsg(msg);
}

void CollisionsPhy::handleLowerMsgStart(cMessage *msg)
{
	increment();
	if (colliding)
		coreEV << "Collision in start\n";
	else	
		BasePhyLayer::handleLowerMsgStart(msg);
}

void CollisionsPhy::handleLowerMsgEnd(cMessage *msg)
{
	if (!colliding)
	{
		//coreEV << "non-colliding message sent up\n";
    	sendUp( decapsMsg(static_cast<AirFrame *>(msg)) );
	}
	else
	{
		coreEV << "Collision! Dropped message\n";
		handleCollision(msg);
	}
	decrement(); 	
}

void CollisionsPhy::handleTransmissionOver()
{
	decrement(); 	
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

