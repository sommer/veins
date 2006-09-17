/***************************************************************************
 * file:        YourDecider.h
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
 **************************************************************************/


#ifndef  YOUR_DECIDER_H
#define  YOUR_DECIDER_H

#include <SnrDecider.h>

class YourDecider : public SnrDecider
{

 public:
     /**
     * If you derive from SnrDecider, you get some useful functions like snrOverThreshold
     * that tell you whether a received signal was strong enough
     */
    Module_Class_Members(YourDecider,SnrDecider,0);
    /**
     * Use this function to read in your parameters
     */
    virtual void initialize(int);

 protected:

    /**
     * @brief In this function the decision whether a frame is received
     * correctly or not is made.
     **/
    virtual void handleLowerMsg(AirFrame*, const SnrList &);
    virtual void handleSelfMsg(cMessage *msg) {
        error("YourDecider does not handle selfmessages");
    }
};
#endif
