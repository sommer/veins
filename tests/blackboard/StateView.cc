/* -*-mode:c++ -*- *********************************************************
 * file:        StateView.cc
 *
 * author:      Andreas Koepke
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistrbute it
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
#include "TestHostState.h"
#include "TestParam.h"
#include "StateChanger.h"

Define_Module( StateView );

void StateView::initialize(int stage)
{
    BaseModule::initialize(stage);
    if(stage == 0) {
        findHost()->subscribe(StateChanger::catHostState, this);
    } else if(stage == 1) {
        scheduleAt(simTime() + 5.0, new cMessage("unsubscribe memo"));
    }
}

void StateView::handleMessage( cMessage* m)
{
    TestParam h;
    findHost()->unsubscribe(StateChanger::catHostState, this);
    ev << "StateView::handleMessage "
       << "unsubscribed HostState" << std::endl;
    findHost()->unsubscribe(StateChanger::catTestParam, this);
    delete m;
}

void StateView::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method("receiveBBItem(%s)", obj->info().c_str());
    const TestHostState *s = dynamic_cast<const TestHostState *>(obj);
    if(s == 0) error("StateView::receiveBBItem could not read details");

    if(s->getState() == TestHostState::DEAD) {
        ev << "StateView::receiveBBItem "
           << "Host state changed to DEAD" << std::endl;
    }
    else if (s->getState() == TestHostState::SLEEP){
        ev << "StateView::receiveBBItem "
           << "Host state changed to SLEEP" << std::endl;
    }
    else if (s->getState() == TestHostState::AWAKE){
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
