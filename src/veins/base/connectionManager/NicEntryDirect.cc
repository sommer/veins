/* -*- mode:c++ -*- ********************************************************
 * file:        NicEntryDirect.cc
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2005 Telecommunication Networks Group (TKN) at
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
 * description: Class to store information about a nic for the
 *              ConnectionManager module
 **************************************************************************/

#include "veins/base/connectionManager/NicEntryDirect.h"
#include "veins/base/connectionManager/ChannelAccess.h"

using std::endl;
using namespace Veins;

void NicEntryDirect::connectTo(NicEntry* other)
{
    cModule* otherPtr = other->nicPtr;

    EV_TRACE << "connecting nic #" << nicId << " and #" << other->nicId << endl;

    cGate* radioGate = nullptr;
    if ((radioGate = otherPtr->gate("radioIn")) == nullptr) throw cRuntimeError("Nic has no radioIn gate!");

    outConns[other] = radioGate->getPathStartGate();
}

void NicEntryDirect::disconnectFrom(NicEntry* other)
{
    EV_TRACE << "disconnecting nic #" << nicId << " and #" << other->nicId << endl;
    outConns.erase(other);
}
