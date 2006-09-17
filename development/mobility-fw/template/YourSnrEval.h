/* -*- mode:c++ -*- ********************************************************
 * file:        YourSnrEval.h
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


#ifndef YOUR_SNREVAL_H
#define YOUR_SNREVAL_H

#include <BasicSnrEval.h>


class YourSnrEval : public BasicSnrEval
{
    Module_Class_Members( YourSnrEval, BasicSnrEval, 0 );

public:
    /** @brief Initialize variables and publish the radio status*/
    virtual void initialize(int);

    /** @brief Called by the Blackboard whenever a change occurs we're interested in */
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);

protected:
    
    /** @brief Buffer the frame and update noise levels and snr information...*/
    virtual void handleLowerMsgStart(AirFrame*);

    /** @brief Unbuffer the frame and update noise levels and snr information*/
    virtual void handleLowerMsgEnd(AirFrame*);
    
    /** @brief Calculate duration of this message -- one of the reasons to
     * have your own SnrEval, as this depends on your MAC */
    virtual double calcDuration(cMessage* m) {
        // you may want to use the bitrate information that you get from the radio
    }
    

protected:

    /** @brief Current state of active channel (radio), set using radio, updated via BB */
    RadioState::States radioState;
    /** @brief category number given by bb for RadioState */
    int catRadioState;
    
    /** @brief Last RSSI level */
    RSSI rssi;
    /** @brief category number given by bb for RSSI */
    int catRSSI;

    /** @brief keep bitrate to calculate duration */
    double bitrate;
    /** @brief BB category of bitrate */
    int catBitrate;
};

#endif
