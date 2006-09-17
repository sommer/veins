/* -*- mode:c++ -*- ********************************************************
 * file:        YourApplLayer.cc
 *
 * author:      Your Name
 *
 * copyright:   (C) 2004 Your Institution
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     Your Simulation
 * description: - Your Description
 ***************************************************************************/


#include "YourApplLayer.h"

Define_Module(YourApplLayer);

// do some initialization 
void YourApplLayer::initialize(int stage)
{
  BasicApplLayer::initialize(stage); //DO NOT DELETE!!

	if(stage==0){
		// ENTER YOUR CODE HERE
		// all subscribe calls should go in the second stage
	}
	else if(stage==1){
		// ENTER YOUR CODE HERE
		// all publish calls have to be made in the first stage
	}
}

// You got a message from the network layer (most likely)
// take care of it
void YourApplLayer::handleLowerMsg( ApplPkt *msg )
{
    // ENTER YOUR CODE HERE
    delete msg;
}

// You have send yourself a message -- probably a timer,
// take care of it
void YourApplLayer::handleSelfMsg(cMessage *msg)
{
    // ENTER YOUR CODE HERE
}

// You got a control message from the network layer
// take care of it
void YourApplLayer::handleLowerControl( cMessage *msg )
{
    // ENTER YOUR CODE HERE
    delete msg;
}


// If you want to use the blackboard uncomment the following functions
// and fill them with functionality
/**
 * Update the internal copies of interesting BB variables
 *
 */
void YourApplLayer::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    int yourCatogory;

    Enter_Method_Silent();
    BasicApplLayer::receiveBBItem(category, details, scopeModuleId);

    if(category == yourCatogory) {
	// ENTER YOUR CODE HERE
    }
}
