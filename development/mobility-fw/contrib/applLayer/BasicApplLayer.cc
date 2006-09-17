/* -*- mode:c++ -*- *******************************************************
 * file:        BasicApplLayer.cc
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
 * description: application layer: general class for the application layer
 *              subclass to create your own application layer
 ***************************************************************************/


#include "BasicApplLayer.h"


Define_Module(BasicApplLayer);


/**
 * First we have to initialize the module from which we derived ours,
 * in this case BasicModule.
 *
 * Then we have to intialize the gates and - if necessary - some own
 * variables.
 **/
void BasicApplLayer::initialize(int stage)
{
    BasicModule::initialize(stage);

    if(stage==0){
	headerLength= par("headerLength");	
	lowergateOut=findGate ("lowergateOut");
	lowergateIn=findGate ("lowergateIn");
        lowerControlIn  = findGate("lowerControlIn");
    }
}

/**
 * The basic handle message function.
 *
 * Depending on the gate a message arrives handleMessage just calls
 * different handle message functions to further process the message.
 *
 * You should not make any changes in this function but implement all
 * your functionality into the handle*Msg functions called from here.
 *
 * @sa handleLowerMsg, handleSelfMsg
 **/
void BasicApplLayer::handleMessage(cMessage *msg)
{
    if(msg->arrivalGateId()==lowergateIn){
        handleLowerMsg(msg);
    }
    else if(msg->arrivalGateId()==lowerControlIn){
        EV << "handle lower control" << endl;
        handleLowerControl(msg);
    }
    else {
        handleSelfMsg(msg);
    }
}

/** 
 * Send message down to lower layer
 **/
void BasicApplLayer::sendDown(cMessage *msg) {
    sendDelayedDown(msg, 0.0);
}

/** 
 * Send message down to lower layer after delay seconds
 **/
void BasicApplLayer::sendDelayedDown(cMessage *msg, double delay) {
    sendDelayed(msg, delay, lowergateOut);
}
