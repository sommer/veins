/* -*- mode:c++ -*- ********************************************************
 * file:        TestHostState.h
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
 * part of:     framework implementation developed by tkn
 * description: Blackboard Parameter that could reflect the state of the host
 ***************************************************************************
 * changelog:   $Revision: 1.4 $
 *              last modified:   $Date: 2004/02/09 13:59:33 $
 *              by:              $Author: omfw-willkomm $
 **************************************************************************/

#ifndef TESTHOSTSTATE_H
#define TESTHOSTSTATE_H

#include <omnetpp.h>

class TestHostState : public cObject
{
public:
    enum States
        {
            SLEEP,
	    DEAD,
	    AWAKE
        };

private:
    States state;

public:

    States getState() const { return state; }
    void setState(States s) { state = s; }
};



#endif
