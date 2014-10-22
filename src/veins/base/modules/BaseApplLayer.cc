/* -*- mode:c++ -*- *******************************************************
 * file:        BaseApplLayer.cc
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


#include "veins/base/modules/BaseApplLayer.h"
#include "veins/base/utils/PassedMessage.h"

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BaseLayer.
 *
 * Then we have to intialize the gates and - if necessary - some own
 * variables.
 **/
void BaseApplLayer::initialize(int stage)
{
    BaseLayer::initialize(stage);
    if(stage==0){
        headerLength= par("headerLength");
    }
}

/**
 * Send message down to lower layer
 **/
void BaseApplLayer::sendDelayedDown(cMessage *msg, simtime_t_cref delay) {
    recordPacket(PassedMessage::OUTGOING, PassedMessage::LOWER_DATA, msg);
    sendDelayed(msg, delay, lowerLayerOut);
}
