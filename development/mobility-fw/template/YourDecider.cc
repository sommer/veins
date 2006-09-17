/***************************************************************************
 * file:        YourDecider.cc
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


#include "YourDecider.h"

#include "AirFrame_m.h"

Define_Module(YourDecider);


void YourDecider::initialize(int stage)
{
    SnrDecider::initialize(stage);
    /**
     * Insert your initialiaztion code here
     */
}

void YourDecider::handleLowerMsg(AirFrame *af, const SnrList& receivedList)
{
    if(snrOverThreshold(receivedList))
    {
        if(af->hasBitError()) {
            EV << "Message got lost ";
        } else {
            EV << "Message handed on to Mac\n";
            sendUp(af->decapsulate());
        }
    }
    else {
        EV << "COLLISION!\n";
    }
    delete af;
}
