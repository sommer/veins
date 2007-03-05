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
#include "ApplPkt_m.h"

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
		EV << "handle lower message" << endl;
		handleLowerMsg(msg);
	} else if (msg->arrivalGateId() == lowerControlIn) {
		EV << "handle lower control" << endl;
		handleLowerControl(msg);
	} else if (msg->arrivalGateId() == applgateIn) {
		EV << "handle application message" << endl;
		handleApplMsg(msg);
	} else if (msg->arrivalGateId() == applControlIn) {
		EV << "handle application control" << endl;
		handleApplControl(msg);
	} else if (msg->arrivalGateId() == locgateIn) {
		EV << "handle localization message" << endl;
		handleLocMsg(msg);
	} else if (msg->arrivalGateId() == locControlIn) {
		EV << "handle localization control" << endl;
		handleLocControl(msg);
	} else {
		handleSelfMsg(msg);
	}
}

void LocFilter::handleSelfMsg(cMessage * msg)
{
	EV << "LocFilter: handleSelfMsg has no idea what to do; delete msg" << endl;
	delete msg;
}

/**
 * @brief Check if message for appl or for loc.
 */
void LocFilter::handleLowerMsg(cMessage * msg)
{
	if (msg->kind() == LOCALIZATION_MSG) {
		send(msg, locgateOut);
	} else {
		send(msg, applgateOut);
	}
}

/**
 * @brief Check if message for appl or for loc.
 */
void LocFilter::handleLowerControl(cMessage * msg)
{
	if (msg->kind() == LOCALIZATION_MSG) {
		send(msg, locControlOut);
	} else {
		send(msg, applControlOut);
	}
}

void LocFilter::handleApplMsg(cMessage * msg)
{
	if (msg->kind() == LOCALIZATION_MSG)
		EV << "LocFilter: Application sending a Localization message????" << endl;

	send(msg, lowergateOut);
}

void LocFilter::handleApplControl(cMessage * msg)
{
	if (msg->kind() == LOCALIZATION_MSG)
		EV << "LocFilter: Application sending a Localization control message????" << endl;

	send(msg, lowerControlOut);
}

void LocFilter::handleLocMsg(cMessage * msg)
{
	if (msg->kind() != LOCALIZATION_MSG)
		EV << "LocFilter: Localization module NOT sending a Localization message????" << endl;

	send(msg, lowergateOut);
}

void LocFilter::handleLocControl(cMessage * msg)
{
	if (msg->kind() != LOCALIZATION_MSG)
		EV << "LocFilter: Localization module NOT sending a Localization control????" << endl;

	send(msg, lowerControlOut);
}
