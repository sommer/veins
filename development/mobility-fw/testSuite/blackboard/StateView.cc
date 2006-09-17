/* -*-mode:c++ -*- *********************************************************
 * file:        StateView.cc
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
 * description: visualizes state subscribed from BB
 ***************************************************************************
 * changelog:   $Revision: 1.3 $
 *              last modified:   $Date: 2004/02/09 13:59:33 $
 *              by:              $Author: omfw-willkomm $
 **************************************************************************/


#include "StateView.h"
#include "HostState.h"
#include "TestParam.h"

Define_Module( StateView );

void StateView::initialize(int stage)
{
    BasicModule::initialize(stage);
    if(stage == 0) {
        HostState s;
        catHostState = bb->subscribe(this, &s, -1);
    } else if(stage == 1) {
        scheduleAt(simTime() + 5.0, new cMessage("unsubscribe memo"));
    }
}
 
void StateView::handleMessage( cMessage* m)
{
    TestParam h;
    bb->unsubscribe(this, catHostState);
    ev << "StateView::handleMessage "
       << "unsubscribed HostState" << std::endl;
    bb->unsubscribe(this, bb->getCategory(&h));
    delete m;
}

void StateView::receiveBBItem(int category, const BBItem *details, int scopeModuleId) 
{
    Enter_Method("receiveBBItem(%s)", details->info().c_str());
    const HostState *s = dynamic_cast<const HostState *>(details);
    if(s == 0) error("StateView::receiveBBItem could not read details");
    
    if(s->getState() == HostState::DEAD) {
        ev << "StateView::receiveBBItem "
           << "Host state changed to DEAD" << std::endl;
    }
    else if (s->getState() == HostState::SLEEP){
        ev << "StateView::receiveBBItem "
           << "Host state changed to SLEEP" << std::endl;
    }
    else if (s->getState() == HostState::AWAKE){
	ev << "StateView::receiveBBItem "
           << "Host state changed to AWAKE" << std::endl;
    } else {
	ev << "StateView::receiveBBItem "
           << " Host changed to unkown state " << std::endl;
    }
}

StateView::~StateView()
{

}
