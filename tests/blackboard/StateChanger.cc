/* -*- mode:c++ -*- ********************************************************
 * file:        StateChanger.cc
 *
 * author:      Andreas Koepke
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
 * part of:     testsuite for framework
 * description: changes host state
 ***************************************************************************
 * changelog:   $Revision: 1.4 $
 *              last modified:   $Date: 2004/02/09 13:59:33 $
 *              by:              $Author: omfw-willkomm $
 **************************************************************************/


#include "StateChanger.h"

Define_Module( StateChanger );

void StateChanger::initialize(int stage)
{
    BaseModule::initialize(stage);
    if(stage == 0) {
        state_counter = 0;
        change_timer = new cMessage("change_timer", 0);
        scheduleAt(simTime() + 1.0, change_timer);
        
        bu = FindModule<BaseUtility*>::findSubModule(this);
        if (bu == NULL)
        	error("Could not find BaseUtility module");
    } else if(stage == 1) {
        catHostState = bu->getCategory(&hs);
        catTestParam = bu->getCategory(&tp);
        hs.setState(HostState::SLEEP);
        tp.setState(TestParam::BLUE);
        bu->publishBBItem(catHostState, &hs, parentModule()->id());
        bu->publishBBItem(catTestParam, &tp, parentModule()->id());
    }
}


void StateChanger::handleMessage(cMessage* msg)
{
    if(msg == change_timer) 
    {
	state_counter++;
	switch(state_counter % 3)
	{
	    case 0:
		hs.setState(HostState::SLEEP);
                tp.setState(TestParam::BLUE);
		break;
	    case 1:
		hs.setState(HostState::DEAD);
                tp.setState(TestParam::RED);
		break;
	    case 2:
		hs.setState(HostState::AWAKE);
		tp.setState(TestParam::GREEN);
		break;
	}
        bu->publishBBItem(catHostState, &hs, parentModule()->id());
        bu->publishBBItem(catTestParam, &tp, parentModule()->id());
	if(state_counter < 10) scheduleAt(simTime() + 1.0, change_timer);
    } else {
	error(" StateChanger::handleMessage got wrong message");
    }
}

  
void StateChanger::finish()
{ 
    if(!change_timer->isScheduled()) delete change_timer;
};
