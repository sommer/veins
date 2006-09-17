/* -*-	mode:c++ -*- *******************************************************
 * file:        TestDecider.cc
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2006 Telecommunication Networks Group (TKN) at
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
 ***************************************************************************/


#include "TestDecider.h"

#include <CoreDebug.h>

Define_Module( TestDecider );

void TestDecider::handleLowerMsg(AirFrame* af, const SnrList & sList)
{
    sendUp( decapsMsg( af ) );
}

void TestDecider::handleSelfMsg(cMessage* msg)
{ 
    coreEV << "TestDecider does not handle self messages\n";
    delete msg;
}
