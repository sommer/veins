/* -*- mode:c++ -*- ********************************************************
 * file:        AlohaMacLayer.cc
 *
 * author:      Marc Loebbers, Yosia Hadisusanto, Andreas Koepke
 *
 * copyright:   (C) 2004,2005,2006
 *              Telecommunication Networks Group (TKN) at Technische
 *              Universitaet Berlin, Germany.
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


#include "AlohaMacLayer.h"
#include "NicControlType.h"
#include "FWMath.h"

Define_Module( AlohaMacLayer )

/**
 * Initialize the of the omnetpp.ini variables in stage 1. In stage
 * two subscribe to the RadioState.
 */
void AlohaMacLayer::initialize(int stage)
{
    BasicMacLayer::initialize(stage);    

    if (stage == 0) {        
        queueLength = hasPar("queueLength") ? par("queueLength") : 10;
	if( queueLength < 1 )
	    error("Minimum queueLength is 1!");
        EV << "queueLength = " << queueLength << endl;
        
        radioState = RadioState::RECV;
        RadioState cs;
        catRadioState = bb->subscribe(this, &cs, parentModule()->id());
        
        // get handle to radio
        radio = SingleChannelRadioAccess().get();

        macState = RX;
    }
    else if(stage == 1) {
        int channel;
        double bitrate = hasPar("bitrate") ? par("bitrate") : 10000;

        channel = hasPar("defaultChannel") ? par("defaultChannel") : 0;

        radio->setActiveChannel(channel);
        radio->setBitrate(bitrate);
    }
}


void AlohaMacLayer::finish() {
    MacQueue::iterator it;
    for(it = macQueue.begin(); it != macQueue.end(); ++it) {
        delete (*it);
    }
    macQueue.clear();
}

/**
 **/
void AlohaMacLayer::handleUpperMsg(cMessage *msg)
{
    MacPkt *mac = encapsMsg(msg);

    // All messages are first put into the queue, if space is available
    if (macQueue.size() <= queueLength) {
	EV << "new packet arrived, put in queue (size=" << macQueue.size() << ")\n";
        macQueue.push_back(mac);
        if((macQueue.size() == 1) && (macState == RX))
	    prepareSend();
    }
    else {
        // queue is full, message has to be deleted
        EV << "New packet arrived, but queue is FULL, so new packet is deleted\n";
        mac->setName("MAC ERROR");
        mac->setKind(NicControlType::PACKET_DROPPED);
        sendControlUp(mac);
    }
}

/**
 * After the timer expires try to retransmit the message by calling
 * handleUpperMsg again.
 */
void AlohaMacLayer::handleSelfMsg(cMessage *msg)
{
    error("AlohaMac does not have self messages");
}


void AlohaMacLayer::handleLowerControl(cMessage *msg)
{
    if(msg->kind() == NicControlType::TRANSMISSION_OVER) {
        EV << " transmission over" << endl;
        macState = RX;
        radio->switchToRecv();
    }
    else {
        EV << "control message with wrong kind -- deleting\n";
    }
    delete msg;
}

void AlohaMacLayer::prepareSend() {
    if(macQueue.size() != 0) {
        EV << " prepare send " << endl;
        if(radio->switchToSend())
	    macState = TX;
    }
}

/**
 * Update the internal copies of interesting BB variables
 *
 */
void AlohaMacLayer::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    BasicMacLayer::receiveBBItem(category, details, scopeModuleId);
    if(category == catRadioState) {
        radioState = static_cast<const RadioState *>(details)->getState();
	EV << " radio switched; macState="<< macState << " (TX="<<TX<<",RX="<<RX<<") radioState=" 
	   << radioState << " (SEND="<<RadioState::SEND<<",RECV="<<RadioState::RECV<<")\n";

        if((macState == TX) && (radioState == RadioState::SEND)) {
            EV << " radio in SEND state, sendDown packet" << endl;
            sendDown(macQueue.front());
            macQueue.pop_front();
        }
        else if((macState == RX) && (radioState == RadioState::RECV)) {
            EV << " radio in RECV state, prepare send" << endl;
            prepareSend();
        }
    }
}
