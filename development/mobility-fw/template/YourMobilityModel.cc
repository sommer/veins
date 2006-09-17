/* -*- mode:c++ -*- ********************************************************
 * file:        YourMobilityModule.cc
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


#include "YourMobilityModule.h"

#define EV (ev.disabled()||!debug) ? ev : ev << logName() << "::YourMobilityModule: "

Define_Module( YourMobilityModule );

// do some initialization
void YourMobilityModule::initialize(int stage)
{
  BasicMobilityModule::initialize(stage); //DO NOT DELETE!!

	if(stage==0){
	  // ENTER YOUR CODE HERE
	  // all publish calls have to be made in the first stage
	}
	else if(stage==1){
	  // ENTER YOUR CODE HERE
	  // all subscribe calls should go in the second stage
	  //You should schedule a self message here to trigger movement
	  //Have a look at ConstSpeedMobility for an example
	}
}

void YourMobilityModule::handleSelfMsg(cMessage *msg)
{
  //ENTER YOUR CODE HERE
  //handle self messages (most likely timer that indicate to take another step)
}

/*
bool YourMobilityModule::blackboardItemChanged(BBItemRef item)
{
	Enter_Method("blackboardItemChanged(\"%s\")", item->label());

	if( BasicMobilityModule::blackboardItemChanged(item) ){
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

bool YourMobilityModule::blackboardItemPublished(BBItemRef item)
{
	Enter_Method("blackboardItemPublished(\"%s\")", item->label());

	if( BasicMobilityModule::blackboardItemPublished(item) ){
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

bool YourMobilityModule::blackboardItemWithdrawn(BBItemRef item)
{
	Enter_Method("blackboardItemWithdrawn(\"%s\")", item->label());

	if( BasicMobilityModule::blackboardItemWithdrawn(item) ){
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
