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
#include "BaseLocAppl.h"
#include "BaseApplLayer.h"
#include <assert.h>

Define_Module(BaseLocalization);

void BaseLocalization::initialize(int stage)
{
	BaseModule::initialize(stage);

	if (stage == 0) {
		headerLength = par("headerLength");
		isAnchor = par("isAnchor");
		lowergateOut = findGate("lowergateOut");
		lowergateIn = findGate("lowergateIn");
		lowerControlIn = findGate("lowerControlIn");
		lowerControlOut = findGate("lowerControlOut");
	} else if (stage == 1) {
		appl = getApplicationModule();
	}
}

void BaseLocalization::handleMessage(cMessage * msg)
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

static cModule* getModule(const char* modname, cModule *top)
{
	for (cSubModIterator i(*top); !i.end(); i++) {
		cModule *submod = i();
		if (strcmp(submod->fullName(), modname) == 0)
			return submod;
	}
	return NULL;
}

BaseLocAppl * BaseLocalization::getApplicationModule() 
{
	cModule *host = findHost();
	BaseApplLayer *layer = static_cast<BaseApplLayer *>(getModule("appl", host));
	if (!layer)
		error("getApplicationModule: no BaseApplLayer module found!");
	BaseLocAppl *appl = static_cast<BaseLocAppl *>(getModule("appl", layer));
	if (!appl)
		error("getApplicationModule: no BaseLocAppl module found!");

	return appl;
}
