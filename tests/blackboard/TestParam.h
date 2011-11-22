/* -*- mode:c++ -*- ********************************************************
 * file:        TestParam.h
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
 * description: Blackboard parameter for test purposes
 ***************************************************************************
 * changelog:   $Revision: 1.3 $
 *              last modified:   $Date: 2004/02/09 13:59:33 $
 *              by:              $Author: omfw-willkomm $
 **************************************************************************/


#ifndef TESTPARAM_H
#define TESTPARAM_H

#include <omnetpp.h>

class TestParam : public cObject
{
public:
    enum States 
        {
            BLUE,
            RED,
            GREEN
        };

private:
    States state;

public:    

    States getState() const { return state; }
    void setState(States s) { state = s; }
    
    TestParam(States s=BLUE) : cObject(), state(s){};
};



#endif
