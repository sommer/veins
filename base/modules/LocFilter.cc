/* -*- mode:c++ -*- ********************************************************
 * file:        LocFilter.cc
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
 * description: this class implements filtering for BaseLocApplLayer
 **************************************************************************/

#include "LocFilter.h"

Define_Module(LocFilter);

void LocFilter::initialize(int stage)
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
		applControlIn = findGate("applControlIn");
		applControlOut = findGate("applControlOut");
		locgateIn = findGate("locgateIn");
		locgateOut = findGate("locgateOut");
		locControlIn = findGate("locControlIn");
		locControlOut = findGate("locControlOut");
	}
}

void LocFilter::handleMessage(cMessage * msg)
{
	if (msg->arrivalGateId() == lowergateIn) {
		handleLowerMsg(msg);
	} else if (msg->arrivalGateId() == lowerControlIn) {
		EV << "handle lower control" << endl;
		handleLowerControl(msg);
	} else if (msg->arrivalGateId() == applgateIn) {
		EV << "handle localization message" << endl;
		handleApplMsg(msg);
	} else if (msg->arrivalGateId() == applControlIn) {
		EV << "handle lower control" << endl;
		handleApplControl(msg);
	} else if (msg->arrivalGateId() == locgateIn) {
		EV << "handle localization message" << endl;
		handleLocMsg(msg);
	} else if (msg->arrivalGateId() == locControlIn) {
		EV << "handle lower control" << endl;
		handleLocControl(msg);
	} else {
		handleSelfMsg(msg);
	}
}

void LocFilter::handleSelfMsg(cMessage * msg)
{
	EV << "BaseLoc: handleSelfMsg has no idea what to do; delete msg\n";
	delete msg;
}

/**
 * @brief Check if message for appl or for loc.
 * Currently it forwards the message to both modules, so the modules
 * have to check themselves if the message was addressed to them.
 * @todo How to filter message?
 */
void LocFilter::handleLowerMsg(cMessage * msg)
{
	cMessage *msg_clone = (cMessage *) msg->dup();
	send(msg, applgateOut);
	send(msg_clone, locgateOut);
}

/**
 * @brief Check if message for appl or for loc.
 * Currently it forwards the message to both modules, so the modules
 * have to check themselves if the message was addressed to them.
 * @todo How to filter message?
 */
void LocFilter::handleLowerControl(cMessage * msg)
{
	cMessage *msg_clone = (cMessage *) msg->dup();
	send(msg, applControlOut);
	send(msg_clone, locControlOut);
}

void LocFilter::handleApplMsg(cMessage * msg)
{
	send(msg, lowergateOut);
}

void LocFilter::handleApplControl(cMessage * msg)
{
	send(msg, lowerControlOut);
}

void LocFilter::handleLocMsg(cMessage * msg)
{
	send(msg, lowergateOut);
}

void LocFilter::handleLocControl(cMessage * msg)
{
	send(msg, lowerControlOut);
}
