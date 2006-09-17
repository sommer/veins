/* -*- mode:c++ -*- ********************************************************
 * file:        TestSnrEval.cc
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
 * description: - SnrEval class
 *              - mains tasks are to determine the SNR for a message and
 *                to simulate a transmission delay
 *
 ***************************************************************************/

#include "TestSnrEval.h"

#include <CoreDebug.h>

Define_Module(TestSnrEval);

double TestSnrEval::calcDuration(cMessage*)
{
    coreEV << "Warning: BasicSnrEval does not provide calculation of the duration of a frame and just return a dummy value!\n";
    return 0.001;
}

void TestSnrEval::handleLowerMsgStart(AirFrame*)
{
    coreEV << "Nothing to do for handleLowerMsgStart\n";
}
