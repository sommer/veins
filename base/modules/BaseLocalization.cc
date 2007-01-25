/* -*- mode:c++ -*- ********************************************************
 * file:        BaseLocalization.cc
 *
 * author:      Peterpaul Klein Haneveld
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
 * description: basic localization class
 *              extend to implement a localization algorithm
 **************************************************************************/

#include "BaseLocalization.h"

Define_Module(BaseLocalization);

void BaseLocalization::initialize(int stage)
{
	BaseModule::initialize(stage);

	if (stage == 0) {
		headerLength = par("headerLength");
		lowergateOut = findGate("lowergateOut");
		lowergateIn = findGate("lowergateIn");
		lowerControlIn = findGate("lowerControlIn");
		lowerControlOut = findGate("lowerControlOut");
		applgateIn = findGate("applgateIn");
		applgateOut = findGate("applgateOut");
	}
}

void BaseLocalization::handleMessage(cMessage * msg)
{
	if (msg->arrivalGateId() == lowergateIn) {
		handleLowerMsg(msg);
	} else if (msg->arrivalGateId() == lowerControlIn) {
		EV << "handle lower control" << endl;
		handleLowerControl(msg);
	} else if (msg->arrivalGateId() == applgateIn) {
		EV << "handle localization message" << endl;
		handleApplMsg(msg);
	} else {
		handleSelfMsg(msg);
	}
}

void BaseLocalization::sendDown(cMessage * msg)
{
	send(msg, lowergateOut);
}

void BaseLocalization::sendDelayedDown(cMessage * msg, double delay)
{
	sendDelayed(msg, delay, lowergateOut);
}

void BaseLocalization::sendControlDown(cMessage * msg)
{
	send(msg, lowerControlOut);
}

void BaseLocalization::sendAppl(cMessage * msg)
{
	send(msg, applgateOut);
}
