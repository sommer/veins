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
#include <assert.h>

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
        if (hasPar("stats") && par("stats").boolValue()) {
            doStats = true;
            passedMsg = new PassedMessage();
            catPassedMsg = utility->getCategory(passedMsg);
            passedMsg->fromModule = id();
            hostId = findHost()->id();
        }
        else {
            doStats = false;
        }
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
    if (msg->isSelfMessage()){
        handleSelfMsg(msg);
    } else if(msg->arrivalGateId()==uppergateIn) {
        recordPacket(PassedMessage::INCOMING,PassedMessage::UPPER_DATA,msg);
        handleUpperMsg(msg);
    } else if(msg->arrivalGateId()==upperControlIn) {
        recordPacket(PassedMessage::INCOMING,PassedMessage::UPPER_CONTROL,msg);
        handleUpperControl(msg);
    } else if(msg->arrivalGateId()==lowerControlIn){
        recordPacket(PassedMessage::INCOMING,PassedMessage::LOWER_CONTROL,msg);
        handleLowerControl(msg);
    } else if(msg->arrivalGateId()==lowergateIn) {
        recordPacket(PassedMessage::INCOMING,PassedMessage::LOWER_DATA,msg);
        handleLowerMsg(msg);
    }
    else if(msg->arrivalGateId()==-1) {
        /* Classes extending this class may not use all the gates, f.e.
         * BaseApplLayer has no upper gates. In this case all upper gate-
         * handles are initialized to -1. When arrivalGateId() equals -1,
         * it would be wrong to forward the message to one of these gates,
         * as they actually don't exist, so raise an error instead.
         */
        opp_error("No self message and no gateID?? Check configuration.");
    } else {
        /* msg->arrivalGateId() should be valid, but it isn't recognized
         * here. This could signal the case that this class is extended
         * with extra gates, but handleMessage() isn't overridden to
         * check for the new gate(s).
         */
        opp_error("Unknown gateID?? Check configuration or override handleMessage().");
    }
}

void BaseLayer::sendDown(cMessage *msg) {
    recordPacket(PassedMessage::OUTGOING,PassedMessage::LOWER_DATA,msg);
    send(msg, lowergateOut);
}

void BaseLayer::sendUp(cMessage *msg) {
    recordPacket(PassedMessage::OUTGOING,PassedMessage::UPPER_DATA,msg);
    send(msg, uppergateOut);
}

void BaseLayer::sendControlUp(cMessage *msg) {
    recordPacket(PassedMessage::OUTGOING,PassedMessage::UPPER_CONTROL,msg);
    send(msg, upperControlOut);
}

void BaseLayer::sendControlDown(cMessage *msg) {
    recordPacket(PassedMessage::OUTGOING,PassedMessage::LOWER_CONTROL,msg);
    send(msg, lowerControlOut);
}

void BaseLayer::recordPacket(PassedMessage::direction_t dir,
                             PassedMessage::gates_t gate,
                             const cMessage * msg) {
    if (!doStats) return;
    passedMsg->direction = dir;
    passedMsg->gateType = gate;
    passedMsg->kind = msg->kind();
    passedMsg->name = msg->name();
    utility->publishBBItem(catPassedMsg, passedMsg, hostId);
}

void BaseLayer::finish() {
    
}

BaseLayer::~BaseLayer()
{
    if (doStats) {
        delete passedMsg;        
    }
}
