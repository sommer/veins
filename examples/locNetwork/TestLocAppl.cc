/***************************************************************************
 * file:        TestLocAppl.cc
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: application layer: test class for the application layer
 ***************************************************************************/

/* ************************************************************************
 * Peterpaul Klein Haneveld:
 **************************************************************************
 * This is a very basic application which only broadcasts its position.
 **************************************************************************/

#include "TestLocAppl.h"

Define_Module_Like(TestLocAppl, BaseLocAppl);

void TestLocAppl::initialize(int stage)
{
	BaseLocAppl::initialize(stage);
	
	switch (stage) {
	case 0:
		delayTimer = new cMessage("delay-timer", SEND_BROADCAST_TIMER);
		break;
	case 1:
		scheduleAt(simTime() + findHost()->index() + 0.005, delayTimer);
		break;
	}
}

void TestLocAppl::handleSelfMsg(cMessage * msg)
{
	switch (msg->kind()) {
	case SEND_BROADCAST_TIMER:
		loc->estimatePosition();
		delete msg;
		delayTimer = NULL;
		break;
	default:
		EV << "Unknown selfmessage! -> delete, kind: " << msg->kind() << endl;
		delete msg;
	}
}

void TestLocAppl::finish()
{
	BaseLocAppl::finish();

	if (delayTimer)
		cancelAndDelete(delayTimer);

}
