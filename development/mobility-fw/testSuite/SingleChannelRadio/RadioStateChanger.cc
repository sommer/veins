/* -*- mode:c++ -*- ********************************************************
 * file:        RadioStateChanger.cc
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


#include "RadioStateChanger.h"

Define_Module( RadioStateChanger );

void RadioStateChanger::initialize(int stage)
{
    BasicModule::initialize(stage);
    if(stage == 0) {
        stateCounter = 0;
        changeTimer = new cMessage("changeTimer", 0);
        scheduleAt(simTime() + 1.0, changeTimer);
    }
    else if(stage == 1) {
        radio = SingleChannelRadioAccess().get();
    }
    
}

void RadioStateChanger::handleMessage(cMessage* msg)
{
    if(msg == changeTimer) 
    {
        stateCounter++;
        stateCounter %= 3;
        if(stateCounter == 0) {
            radio->switchToSleep();
        }
        else if(stateCounter == 1) {
            radio->switchToRecv();
        }
        else if(stateCounter == 2) {
            radio->switchToSend();
        }
        if(stateCounter < 10) scheduleAt(simTime() + 1.0, changeTimer);
    } else {
	error(" RadioStateChanger::handleMessage got wrong message");
    }
}

void RadioStateChanger::finish() 
{
    if(!changeTimer->isScheduled()) delete changeTimer;
}
