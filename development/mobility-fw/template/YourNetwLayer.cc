/* -*- mode:c++ -*- ********************************************************
 * file:        YourNetwLayer.cc
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
 *
 ***************************************************************************
 * changelog:   $Revision: 139 $
 *              last modified:   $Date: 2005-05-27 09:05:43 +0200 (Fr, 27 Mai 2005) $
 *              by:              $Author: koepke $
 ***************************************************************************/


#include "YourNetwLayer.h"

#define EV (ev.disabled()||!debug) ? ev : ev << logName() << "::YourNetwLayer: "

Define_Module(YourNetwLayer);

// do some initialization
void YourNetwLayer::initialize(int stage)
{
  BasicNetwLayer::initialize(stage); //DO NOT DELETE!!

	if(stage==0){
		// ENTER YOUR CODE HERE
		// all publish calls have to be made in the first stage
	}
	else if(stage==1){
		// ENTER YOUR CODE HERE
		// all subscribe calls should go in the second stage
	}
}

// You send yourself a message -- probably a timer,
// take care of it here
void YourNetwLayer::handleSelfMsg(cMessage*)
{
    // ENTER YOUR CODE HERE
}

// You got a message from the MAC layer (most likely)
// take care of it, e.g. check whether you are the final
// receiver and so on
void YourNetwLayer::handleLowerMsg( NetwPkt *msg )
{
    // ENTER YOUR CODE HERE

    // pass encapsulated application message to
    // application layer
    sendUp(msg);
}

// You got a message from an application (most likely)
// It is your job to figure out how to forward it

void YourNetwLayer::handleUpperMsg(NetwPkt* msg)
{
  int nextHop;
    // ENTER YOUR CODE HERE -- esp. specify the next hop

    // pass the prepared message down the the MAC layer that it can
    // take care of the actual one hop transmission
    //include the hop as a parameter, and specify the gate (can be
    //left as it is)
    sendDown(msg, nextHop, lowergateOut);
}

// If you want to use the blackboard uncomment the following functions
// and fill them with functionality

/*
bool YourNetwLayer::blackboardItemChanged(BBItemRef item)
{
	Enter_Method("blackboardItemChanged(\"%s\")", item->label());

	if( BasicNetwLayer::blackboardItemChanged(item) ){
		print("item already handled by parent class");
		return true;
	}

	if( item == yourItem ){ // if this is the item you want to handle
		// ENTER YOUR CODE HERE
		return true;
	}

	// return false if item was not handled
	return false;
}

bool YourNetwLayer::blackboardItemPublished(BBItemRef item)
{
	Enter_Method("blackboardItemPublished(\"%s\")", item->label());

	if( BasicNetwLayer::blackboardItemPublished(item) ){
	   print("item already handled by parent class");
	   return true;
	}

	if( item == yourItem ){ // if this is the item you want to handle
	  // ENTER YOUR CODE HERE
		return true;
	}

	// return false if item was not handled
	return false;
}

bool YourNetwLayer::blackboardItemWithdrawn(BBItemRef item)
{
	Enter_Method("blackboardItemWithdrawn(\"%s\")", item->label());

	if( BasicNetwLayer::blackboardItemWithdrawn(item) ){
	   print("item already handled by parent class");
	   return true;
	}

	if( item == yourItem ){ // if this is the item you want to handle
	  // ENTER YOUR CODE HERE
	  return true;
	}

	// return false if item was not handled
	return false;
}
*/
