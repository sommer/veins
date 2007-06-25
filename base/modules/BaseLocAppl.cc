/* -*- mode:c++ -*- ********************************************************
 * file:        BaseLocAppl.cc
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
 * description: basic localization application class
 *              extend to create localization algorithm
 **************************************************************************/

#include "BaseLocAppl.h"
#include "BaseLocalization.h"
#include "BaseApplLayer.h"
#include <assert.h>

Define_Module(BaseLocAppl);

void BaseLocAppl::initialize(int stage)
{
	BaseModule::initialize(stage);

	if (stage == 0) {
		headerLength = par("headerLength");
		lowergateOut = findGate("lowergateOut");
		lowergateIn = findGate("lowergateIn");
		lowerControlIn = findGate("lowerControlIn");
		lowerControlOut = findGate("lowerControlOut");
	} else if (stage == 1) {
		loc = getLocalizationModule();
	}
}

void BaseLocAppl::handleMessage(cMessage * msg)
{
	if (msg->arrivalGateId() == lowergateIn) {
		handleLowerMsg(msg);
	} else if (msg->arrivalGateId() == lowerControlIn) {
		EV << "handle lower control" << endl;
		handleLowerControl(msg);
	} else {
		handleSelfMsg(msg);
	}
}

void BaseLocAppl::sendDown(cMessage * msg)
{
	send(msg, lowergateOut);
}

void BaseLocAppl::sendDelayedDown(cMessage * msg, double delay)
{
	sendDelayed(msg, delay, lowergateOut);
}

void BaseLocAppl::sendControlDown(cMessage * msg)
{
	send(msg, lowerControlOut);
}

static cModule* getModule(const char* modname, cModule *top)
{
	for (cSubModIterator i(*top); !i.end(); i++) {
		cModule *submod = i();
		if (strcmp(submod->fullName(), modname) == 0)
			return submod;
	}
	return NULL;
}

BaseLocalization * BaseLocAppl::getLocalizationModule() 
{
	cModule *host = findHost();
	BaseApplLayer *layer = static_cast<BaseApplLayer *>(getModule("appl", host));
	if (!layer)
		error("getLocalizationModule: no BaseApplLayer module found!");
	BaseLocalization *loc = static_cast<BaseLocalization *>(getModule("loc", layer));
	if (!loc)
		error("getLocalizationModule: no BaseLocalization module found!");

	return loc;
}
