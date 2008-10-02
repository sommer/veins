/* -*- mode:c++ -*- ********************************************************
 * file:        BasePacket.cc
 *
 * author:      Tom Parker
 *
 * copyright:   (C) 2006 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     message classes
 * description: packet utility class
 ***************************************************************************/
#include "BasePacket.h"

std::string BasePacket::logName(void) const {
	std::ostringstream ost;
	cModule *parent = findHost();
	parent->hasPar("logName") ?
		ost << parent->par("logName").stringValue() : ost << parent->getName();
	ost << "[" << parent->getIndex() << "]";
	return ost.str();
};

//TODO: consider removing this method because simTime() is a global method in omnet4
simtime_t BasePacket::simTime () const
{
	return simTime();
}

cModule *BasePacket::findHost(void) const 
{
	cModule *mod=dynamic_cast<cModule*>(getOwner());
	if (!mod)
		opp_error("findHost: no owner!");
    for (; mod != NULL; mod = mod->getParentModule())
    {
        if (strstr(mod->getName(), "node") != NULL || strstr(mod->getName(), "Node") != NULL)
            break;
    }
    if (!mod)
        opp_error("findHost: no host module found!");

    return mod;
}

