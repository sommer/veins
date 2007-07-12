/* -*- mode:c++ -*- ********************************************************
 * file:        BasePropagation.cc
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
 * part of:     base modules
 * description: propagation layer: default single cell model
 ***************************************************************************/

#include "BasePropagation.h"
#include "BasePhyLayer.h"
#include <assert.h>
#include "FindModule.h"

Define_Module(BasePropagation);

void BasePropagation::initialize(int stage)
{
	BaseModule::initialize(stage);
	if (stage == 0)
	{
		nodes = new NodeList();
        hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;
		if (!hasPar("logName"))
			addPar("logName")->setStringValue(name());
	}
}

BasePropagation::~BasePropagation()
{
	delete nodes;
}


void BasePropagation::sendToChannel(BasePhyLayer *phy,AirFrame *msg)
{
	//Enter_Method_Silent();
	//coreEV << "node number "<<phy->getNode()->index()<<" sending a message"<<endl;
	if (nodes->begin() == nodes->end())
		error("No nodes to talk to!");
	for (NodeList::iterator i = nodes->begin();i!=nodes->end();i++)
	{
		if (*i ==phy)
			continue;
		cMessage *n = static_cast<cMessage*>(msg->dup());
		//coreEV << "sending message to "<<(*i)->getNode()->index()<<endl;
		phy->sendDirect(n,0.0,*i,INGATE);
		//assert(0);
	}
	delete msg;
}

void BasePropagation::registerNic( BasePhyLayer * phy)
{
	coreEV << "Registered nic for node "<<phy->getNode()->index()<<endl;

    // create a new gate for the phy module
    phy->addGate(INGATE,'I');
	nodes->push_back(phy);
}

