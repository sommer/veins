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


#include "NicEntryDirect.h"
#include "ChannelAccess.h"

#ifndef nicEV
#define nicEV (ev.isDisabled()||!coreDebug) ? ev : ev << "NicEntry: "
#endif


/**
 * Establish unidirectional connection with other nic
 *
 * @param other reference to remote nic (other NicEntry)
 *
 * This function acquires an in gate at the remote nic and an out
 * gate at this nic, connects the two and updates the freeInGate,
 * freeOutGate and outConns data sets.
 *
 * It handles compound modules correctly, provided that the physical
 * module is called "phy" or "snrEval" respectively in the .ned files.
 **/
void NicEntryDirect::connectTo(NicEntry* other)
{
    cModule* otherPtr = other->nicPtr;

    nicEV <<"connecting nic #"<<nicId<< " and #"<<other->nicId<<endl;

    cGate *radioGate=NULL;
    if( (radioGate = otherPtr->gate("radioIn")) == NULL )
		throw cRuntimeError("Nic has no radioIn gate!");

    outConns[other] = radioGate->getPathStartGate();
}

/**
 * Release unidirectional connection with other nic
 *
 * @param other reference to remote nic (other NicEntry)
 **/
void NicEntryDirect::disconnectFrom(NicEntry* other)
{
    nicEV <<"disconnecting nic #"<<nicId<< " and #"<<other->nicId<<endl;
    outConns.erase(other);
}
