/* -*- mode:c++ -*- ********************************************************
 * file:        RadioStateChanger.h
 *
 * author:      Andreas Koepke
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
 * part of:     testsuite of framework
 * description: changes radio state
 ***************************************************************************
 * changelog:   $Revision: 1.3 $
 *              last modified:   $Date: 2004/02/09 13:59:33 $
 *              by:              $Author: omfw-willkomm $
 **************************************************************************/


#ifndef RADIO_STATE_CHANGER_H
#define RADIO_STATE_CHANGER_H

#include <omnetpp.h>
#include <BasicModule.h>
#include <SingleChannelRadio.h>

class RadioStateChanger : public BasicModule
{
private:
    cMessage *changeTimer;
    SingleChannelRadio *radio;
    int stateCounter;
    
public:
    Module_Class_Members(RadioStateChanger, BasicModule, 0);
    virtual void initialize(int);
    virtual void handleMessage(cMessage*);
    virtual void finish();
};

#endif
 
