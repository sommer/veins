/* -*- mode:c++ -*- ********************************************************
 * file:        TestCCConnApplLayer.cc
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
 * part of:     mobility framework
 * description: application layer to test connections established by
 *              channelcontrol module
 ***************************************************************************
 * changelog:   $Revision: 266 $
 *              last modified:   $Date: 2006-04-28 16:14:49 +0200 (Fr, 28 Apr 2006) $
 *              by:              $Author: koepke $
 **************************************************************************/


#include "TestCCConnApplLayer.h"

Define_Module_Like( TestCCConnApplLayer, BaseApplLayer );

void TestCCConnApplLayer::initialize(int stage)
{
    BaseApplLayer::initialize(stage);
    if(stage==0){
        if((myApplAddr() == 0) || (myApplAddr() == 1))
            scheduleAt(simTime() + 1.0 + myApplAddr(), new cMessage(0,10));
    }
}

void TestCCConnApplLayer::handleLowerMsg(cMessage* msg)
{
    ev<<"got broadcast message from "<<check_and_cast<ApplPkt* >(msg)->getSrcAddr()<<endl;
    delete msg;
}

void TestCCConnApplLayer::handleSelfMsg(cMessage* msg)
{
    // we should send a broadcast packet ... 
    ApplPkt* m = new ApplPkt;
    m->setDestAddr(-1);
    m->setSrcAddr(myApplAddr());
    m->setLength(headerLength);
    ev<<"Sending broadcast packet!"<<endl;
    sendDown(m);
    delete msg;
}
