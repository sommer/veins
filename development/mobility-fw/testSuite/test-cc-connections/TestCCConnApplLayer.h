/* -*- mode:c++ -*- ********************************************************
 * file:        TestCCConnApplLayer.h
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
 * part of:     mobility framework
 * description: application layer to test connections established by
 *              channelcontrol module
 ***************************************************************************
 * changelog:   $Revision: 266 $
 *              last modified:   $Date: 2006-04-28 16:14:49 +0200 (Fr, 28 Apr 2006) $
 *              by:              $Author: koepke $
 **************************************************************************/


#ifndef TESTCCCONNAPPL_LAYER_H
#define TESTCCCONNAPPL_LAYER_H

#include <BasicApplLayer.h>

class TestCCConnApplLayer : public BasicApplLayer
{
public:
    Module_Class_Members(TestCCConnApplLayer, BasicApplLayer, 0);
    virtual void initialize(int);
protected:
    virtual void handleLowerMsg( cMessage* );
    virtual void handleSelfMsg( cMessage* );
};

#endif
 
