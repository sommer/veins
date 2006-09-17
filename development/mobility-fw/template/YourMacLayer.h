/* -*- mode:c++ -*- ********************************************************
 * file:        YourMacLayer.h
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


#ifndef YOURMAC_LAYER_H
#define YOURMAC_LAYER_H

/**
 * A MAC protocol is fairly complex -- so is the template. The MAC protocol
 * has to take care of many things: it has to switch the radio into proper
 * states, take care of the bit rate at which it sends etc. 
 */

#include <BasicMacLayer.h> 
#include <Blackboard.h>

/** you may need to modify the information in it */
#include <MacPkt_m.h>

/** set the channel on which we send */
#include <ActiveChannel.h>
/** switch the radio into the appropriate states */
#include <RadioState.h>
/** set the bitrate at which the radio transmits */
#include <Bitrate.h>

/** for carrier sense: track the RSSI to figure out whether the channel is busy */
#include <RSSI.h>


/** which radio do we use? */
#include <SingleChannelRadio.h>

/** any fancy address scheme? */
#include <SimpleAddress.h>

class  YourMacLayer : public BasicMacLayer
{
  public:
    Module_Class_Members(YourMacLayer, BasicMacLayer, 0);

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

    /** @brief Delete all dynamically allocated objects of the module*/
    virtual void finish();

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage*);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage*);

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage *msg);

    /** @brief Called by the Blackboard whenever a change occurs we're interested in */
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);

protected:
    /** @brief Current state of active channel (radio), set using radio, updated via BB */
    RadioState::States radioState;
    /** @brief category number given by bb for RadioState */
    int catRadioState;
    
    /** @brief Last RSSI level */
    double rssi;
    /** @brief category number given by bb for RSSI */
    int catRSSI;

    /** @brief RSSI level where medium is considered busy */
    double busyRSSI;

    /** @brief the bit rate at which we transmit */
    double bitrate;

    /** and then of course your functions */
};


#endif
 
