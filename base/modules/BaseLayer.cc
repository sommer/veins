/***************************************************************************
 * file:        BaseLayer.cc
 *
 * author:      Andreas Koepke
 *
 * copyright:   (C) 2006 Telecommunication Networks Group (TKN) at
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
 * description: basic MAC layer class
 *              subclass to create your own MAC layer
 ***************************************************************************
 * changelog:   $Revision: 250 $
 *              last modified:   $Date: 2006-04-04 18:53:02 +0200 (Tue, 04 Apr 2006) $
 *              by:              $Author: koepke $
 **************************************************************************/


#include "BaseLayer.h"

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BaseModule.
 * This module takes care of the gate initialization.
 *
 **/
void BaseLayer::initialize(int stage)
{
    BaseModule::initialize(stage);
    if(stage==0){
        uppergateIn  = findGate("uppergateIn");
        uppergateOut = findGate("uppergateOut");
        lowergateIn  = findGate("lowergateIn");
        lowergateOut = findGate("lowergateOut");
        upperControlIn  = findGate("upperControlIn");
        upperControlOut = findGate("upperControlOut");
        lowerControlIn  = findGate("lowerControlIn");
        lowerControlOut = findGate("lowerControlOut");
    }
}


/**
 * The basic handle message function.
 *
 * Depending on the gate a message arrives handleMessage just calls
 * different handle*Msg functions to further process the message.
 *
 * You should not make any changes in this function but implement all
 * your functionality into the handle*Msg functions called from here.
 *
 * @sa handleUpperMsg, handleLowerMsg, handleSelfMsg
 **/
void BaseLayer::handleMessage(cMessage* msg)
{
    if(msg->arrivalGateId()==uppergateIn) {
        handleUpperMsg(msg);
    }
    else if(msg->arrivalGateId()==lowergateIn){
        handleLowerMsg(msg);
    }
    else if(msg->arrivalGateId()==lowerControlIn){
        handleLowerControl(msg);
    }
    else {
        handleSelfMsg(msg);
    }
}

void BaseLayer::sendDown(cMessage *msg) {
    send(msg, lowergateOut);
}

void BaseLayer::sendUp(cMessage *msg) {
    send(msg, uppergateOut);
}

void BaseLayer::sendControlUp(cMessage *msg) {
    send(msg, upperControlOut);
}

void BaseLayer::sendControlDown(cMessage *msg) {
    send(msg, lowerControlOut);
}
