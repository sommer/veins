/* -*- mode:c++ -*- *******************************************************
 * file:        BasicDecider.cc
 *
 * author:      Marc Loebbers
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
 ***************************************************************************/


#include "BasicDecider.h"

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BasicModule.
 *
 * Then we have to intialize the gates and - if necessary - some own
 * variables.
 *
 */
void BasicDecider::initialize(int stage)
{
    BasicModule::initialize(stage);
  
    if (stage == 0){
        uppergateOut = findGate("uppergateOut");
        lowergateIn = findGate("lowergateIn");
    }
}

/**
 * The basic handle message function.
 *
 * Depending on the gate a message arrives handleMessage just calls
 * different handle*Msg functions to further process the message.
 *
 * The decider module only handles messages from lower layers. All
 * messages from upper layers are directly passed to the snrEval layer
 * and cannot be processed in the decider module
 *
 * You should not make any changes in this function but implement all
 * your functionality into the handle*Msg functions called from here.
 *
 * @sa handleLowerMsg, handleSelfMsg
 */
void BasicDecider::handleMessage(cMessage *msg)
{
    if (msg->arrivalGateId() == lowergateIn) {
        AirFrame *m = static_cast<AirFrame *>(msg);
        SnrControlInfo *cInfo = static_cast<SnrControlInfo *>(m->removeControlInfo());
        // read in the snrList from the control info
        handleLowerMsg(m, cInfo->getSnrList());
        // delete the control info
        delete cInfo;
    }
    else if (msg->isSelfMessage()){
        handleSelfMsg(msg);
    }
}

/**
 * 
 **/
cMessage* BasicDecider::decapsMsg(AirFrame* frame) 
{
    cMessage *m = frame->decapsulate();

    // delete the AirFrane
    delete frame;

    // retrun the MacPkt
    return m;
}

/**
 * Send message to the upper layer.  
 *
 * to be called within @ref handleLowerMsg.
 */
void BasicDecider::sendUp(cMessage *msg)
{
    send(msg, uppergateOut);
}
