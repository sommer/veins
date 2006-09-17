/***************************************************************************
 * file:        YourSnrEval.cc
 *
 * author:      Your Name
 *
 * copyright:   (C) 2004 Your Institute
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


#include "YourSnrEval.h"

#include "Bitrate.h"
#include <FWMath.h>
#include "NicControlType.h"

#include <ChannelControl.h>

Define_Module(YourSnrEval);

/**
 * All values not present in the ned file will be read from the
 * ChannelControl module or assigned default values.
 **/
void YourSnrEval::initialize(int stage)
{
    BasicSnrEval::initialize(stage);
  
    if(stage==0){
        
        radioState = RadioState::RECV;

        // subscribe for information about the radio
        RadioState cs;
        Bitrate br;
        
        catRadioState = bb->subscribe(this, &cs, parentModule()->id());
        catRSSI = bb->getCategory(&rssi);

        catBitrate = bb->subscribe(this, &br, parentModule()->id());        
    }
    else if(stage == 1) {
        /** you may want to publish RSSI information here */
    }
}

/**
 * This function is called right after a packet arrived, i.e. right
 * before it is buffered for 'transmission time'.
 **/
void YourSnrEval::handleLowerMsgStart(AirFrame *frame)
{
    /** you may want to publish RSSI information here */
}


/**
 * This function is called right after the transmission is over,
 * i.e. right after unbuffering.
 *
 * First check the current radio state. The radio must not be switched from RECV
 * state before the end of message is received. Otherwise drop the message.
 * Additionally the snr information of the currently being received message (if any)
 * has to be updated with the receivetime as timestamp and a new snr value.
 * The new SnrList and the AirFrame are sent to the decider.
 *
 **/
void YourSnrEval::handleLowerMsgEnd(AirFrame *frame)
{
    /** you may want to publish RSSI information here */
}

/**
 * These informations are very basic for any SnrEval for a single channel, half duplex radio.
 * You can not receive and transmit at the same time, so...
 */
void YourSnrEval::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    BasicSnrEval::receiveBBItem(category, details, scopeModuleId);    
    if(category == catRadioState) {
        radioState = static_cast<const RadioState *>(details)->getState();
        if(radioState == RadioState::RECV) {
            EV <<"Radio switched to RECV at T= "<<simTime()<<endl;
            bb->publishBBItem(catRSSI, &rssi, nicModuleId);
        }
    }
    else if(category == catBitrate) {
        bitrate = static_cast<const Bitrate *>(details)->getBitrate();
    }
}
