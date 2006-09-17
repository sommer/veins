/* -*- mode:c++ -*- ********************************************************
 * file:        SingleChannelRadio.cc
 *
 * author:      Andreas Koepke
 *
 * copyright:   (C) 2005 Telecommunication Networks Group (TKN) at
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
 ***************************************************************************/

#include "SingleChannelRadio.h"

Define_Module(SingleChannelRadio);

void SingleChannelRadio::initialize(int stage)
{
    BasicModule::initialize(stage);
    if(stage == 0) {
        
        state = RadioState(RadioState::RECV);
        aChannel = ActiveChannel(0);
        
        if(hasPar("swSleep")) {
            swSleep = par("swSleep").doubleValue();
        } else {
            swSleep = 0.0;
        }
        EV<< "swSleep duration = "<< swSleep<< endl;

        if(hasPar("swSend")) {
            swSend = par("swSend").doubleValue();
        }
        else {
            swSend = 0.0;
        }
        EV<< "swSend duration = "<< swSend<< endl;

        if(hasPar("swRecv")) {
            swRecv = par("swRecv").doubleValue();
        }
        else {
            swRecv = 0.0;
        }
        EV<< "swRecv duration = "<< swRecv<< endl;
        
        // initialize the timer
        timer = new cMessage("SwitchTimer");
        nicModuleId = parentModule()->id();

        // get channel category, default channel is set by MAC
        aChannelCat = bb->getCategory(&aChannel);

        // get bit rate category, default bit rate is set by MAC
        bitrateCat = bb->getCategory(&bitrate);
        
        // define state
        stateCat = bb->getCategory(&state);

    }
    else if(stage == 1) {
        state.setState(RadioState::RECV);
        bb->publishBBItem(stateCat, &state, nicModuleId);
        handlingTimer = false;
    }
}

void SingleChannelRadio::handleMessage(cMessage* m) 
{
    if(m == timer) {
        handlingTimer = true;
        switch(state.getState()) {
        case RadioState::SWITCH_TO_SLEEP:
            state.setState(RadioState::SLEEP);
            break;
        case RadioState::SWITCH_TO_SEND:
            state.setState(RadioState::SEND);
            break;
        case RadioState::SWITCH_TO_RECV:
            state.setState(RadioState::RECV);
            break;
        default:
            error("SCRadio::handleMessage radio in stupid state %i",
                  state.getState());
            break;
        }
        handlingTimer = false;
        bb->publishBBItem(stateCat, &state, nicModuleId);
    }
    else {
        error("SCRadio::handleMessage called -- but does not understand it");
    }
}

bool SingleChannelRadio::switchToSleep() 
{
    if(debug) {
        Enter_Method("switchToSleep");
    } else {
        Enter_Method_Silent();
    }
    return switchTo(RadioState::SWITCH_TO_SLEEP, swSleep);
}

bool SingleChannelRadio::switchToSend() 
{
    if(debug) {
        Enter_Method("switchToSend");
    } else {
        Enter_Method_Silent();
    }
    return switchTo(RadioState::SWITCH_TO_SEND, swSend);
}


bool SingleChannelRadio::switchToRecv()
{
    if(debug) {
        Enter_Method("switchToRecv");
    } else {
        Enter_Method_Silent();
    }
    return switchTo(RadioState::SWITCH_TO_RECV, swRecv);
}

bool SingleChannelRadio::setActiveChannel(int c) 
{
    if(debug) {
        Enter_Method("setActiveChannel %i", c);
    } else {
        Enter_Method_Silent();
    }
    aChannel.setActiveChannel(c);
    bb->publishBBItem(aChannelCat, &aChannel, nicModuleId);
    return true;
}

bool SingleChannelRadio::setBitrate(double b) 
{
    if(debug) {
        Enter_Method("setBitRate %g", b);
    } else {
        Enter_Method_Silent();
    }
    bitrate.setBitrate(b);
    bb->publishBBItem(bitrateCat, &bitrate, nicModuleId);
    return true;
}

bool SingleChannelRadio::switchTo(RadioState::States s, simtime_t delta) 
{
    bool success = true;

    EV << "current state:" << state.getState() << ", last state:" << RadioState::SWITCH_TO_SLEEP
       << " handlingTimer:" << handlingTimer << endl;
    if((state.getState() < RadioState::SWITCH_TO_SLEEP) && (handlingTimer == false))
    {
        state.setState(s);
        scheduleAt(simTime() + delta, timer);
        bb->publishBBItem(stateCat, &state, nicModuleId);
    } else {
	EV << "error, radio couldn't be switched!\n";
        success = false;
    }
    return success;
}

void SingleChannelRadio::finish() 
{
    BasicModule::finish();
    if(!timer->isScheduled()) delete timer;
}
