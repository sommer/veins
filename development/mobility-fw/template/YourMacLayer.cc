/* -*- mode:c++ -*- ********************************************************
 * file:        YourMacLayer.cc
 *
 * author:      Your Name
 *
 * copyright:   (C) 2004 Your Institution
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     Your Simulation
 * description: - Your Description
 *
 ***************************************************************************
 * changelog:   $Revision: 382 $
 *              last modified:   $Date: 2006-07-31 16:10:24 +0200 (Mo, 31 Jul 2006) $
 *              by:              $Author: koepke $
 ***************************************************************************/


#include "YourMacLayer.h"


#include "YourMacLayer.h"
#include "NicControlType.h"
#include "FWMath.h"

Define_Module( YourMacLayer )

/**
 * Initialize the of the omnetpp.ini variables in stage 1. In stage
 * two subscribe to the RadioState.
 */
void YourMacLayer::initialize(int stage)
{
    BasicMacLayer::initialize(stage);    

    if (stage == 0) {
        busyRSSI = hasPar("busyRSSI") ? par("busyRSSI") : -90;
        bitrate = hasPar("bitrate") ? par("bitrate") : 10000;

        radioState = RadioState::RECV;
        rssi = 0;

        RadioState cs;
        RSSI rs;

        /** subscribe to the information that is important for you */
        catRadioState = bb->subscribe(this, &cs, parentModule()->id());
        catRSSI = bb->subscribe(this, &rs, parentModule()->id());
        
        // get handle to radio
        radio = SingleChannelRadioAccess().get();

        // and more stuff that needs to be done
    }
    else if(stage == 1) {
        channel = hasPar("defaultChannel") ? par("defaultChannel") : 0;
        /** now tell the radio about it */
        radio->setActiveChannel(channel);
        radio->setBitrate(bitrate);
        busyRSSI = FWMath::dBm2mW(busyRSSI);
    }
}


void YourMacLayer::finish() {
    // deallocate anything that you allocated in initialize
}

/** upper service access point */

void YourMacLayer::handleUpperMsg(cMessage *msg)
{
    MacPkt *mac = encapsMsg(msg);
    /** now decide what to do with it put it in a queue? Check whether we can send it immediatley? */
}

/** Probably some timer expired -- do something about it  */
void YourMacLayer::handleSelfMsg(cMessage *msg) {

}


/** we got something from the radio */
void YourMacLayer::handleLowerMsg(cMessage *msg)
{
    MacPkt *mac = static_cast<MacPkt *>(msg);
    int dest = mac->getDestAddr();
    
    if(dest == myMacAddr || dest == L2BROADCAST)
    {
        EV << "sending pkt to upper...\n";
        sendUp(decapsMsg(mac));
    }
    else {
        EV << "packet not for me, deleting...\n";
	delete mac;
    }
    /** of course, this is just an example. Your MAC has to do more about it */
}

/** There are also some control messages, most notably the indication that a
 * packet transmission is over */
void YourMacLayer::handleLowerControl(cMessage *msg)
{
    if(msg->kind() == NicControlType::TRANSMISSION_OVER) {
        EV << " transmission over" << endl;
        radio->switchToRecv();
    }
    else {
        EV << "control message with wrong kind -- deleting\n";
    }
    /** as always: this is not functional -- you need to do more here */
    delete msg;
}


/**
 * Update the internal copies of interesting BB variables
 *
 */
void YourMacLayer::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    Enter_Method_Silent(); // for debugging you may want to use the loud Enter_Method macro.
    BasicMacLayer::receiveBBItem(category, details, scopeModuleId);

    if(category == catRadioState) {
        radioState = static_cast<const RadioState *>(details)->getState();
        /** now what? The radio is now in a different state */
    }
    else if(category == catRSSI) {
        rssi = static_cast<const RSSI *>(details)->getRSSI();
        if(radioState == RadioState::RECV) {
             /** now what? The channel may be in a different state */
        }
    }
}


