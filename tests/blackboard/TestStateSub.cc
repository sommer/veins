/* -*-mode:c++ -*- *********************************************************
 * file:        TestStateSub.cc
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


#include "TestStateSub.h"
#include "TestParam.h"
#include "StateChanger.h"

Define_Module( TestStateSub );

void TestStateSub::initialize(int stage)
{
    BaseModule::initialize(stage);
    if(stage == 0) {
        findHost()->subscribe(StateChanger::catTestParam, this);
    } else if(stage == 1) {
        scheduleAt(simTime() + 6.0, new cMessage("unsubscribe memo"));
    }
}
 
void TestStateSub::handleMessage( cMessage* m)
{
	findHost()->unsubscribe(StateChanger::catTestParam, this);
    ev << "TestStatSub::handleMessage "
       << "unsubscribed TestParam" << std::endl;
    delete m;
}

void TestStateSub::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method("receiveBBItem(%s)", obj->info().c_str());
    const TestParam *s = dynamic_cast<const TestParam *>(obj);
    if(s == 0) error("TestStateSub::receiveBBItem could not read details");

    if(s->getState() == TestParam::RED) {
        ev << "TestStateSub::receiveBBItem "
           << "test param changed to RED" << std::endl;
    }
    else if (s->getState() == TestParam::GREEN){
        ev << "TestStateSub::receiveBBItem "
           << "test param changed to GREEN" << std::endl;
    }
    else if (s->getState() == TestParam::BLUE){
        ev << "TestStateSub::receiveBBItem "
           << "test param changed to BLUE" << std::endl;
    } else {
	ev << "TestStateSub::receiveBBItem "
           << " Host changed to unkown state " << std::endl;
    }
}
