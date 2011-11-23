/* -*- mode:c++ -*- ********************************************************
 * file:        BaseArp.cc
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
 ***************************************************************************/

#include "BaseArp.h"
#include "FindModule.h"

Define_Module(BaseArp);

void BaseArp::initialize(int stage)
{
    BaseModule::initialize(stage);
    if(stage==0) {
        hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;
	}
}

LAddress::L2Type BaseArp::getMacAddr(const LAddress::L3Type& netwAddr) const
{
    Enter_Method("getMacAddr(%d)",netwAddr);
    cModule *const netwLayer = simulation.getModule( static_cast<int>(netwAddr) );
    if(!netwLayer) {
    	opp_error("Invalid network address: %d! Could not find a module with "
				  "that id.", netwAddr);
    }
    cModule *const pNetwlHost = FindModule<cModule*>::findHost(netwLayer);
    if (pNetwlHost) {
    	const cModule *const pNic = pNetwlHost->getSubmodule( "nic" );
		LAddress::L2Type macAddr( pNic ? pNic->getId() : pNetwlHost->getId() );
    	if (pNic) {
			coreEV << "for host[" << pNetwlHost->getIndex()
			       << "]: netwAddr " << netwAddr << "; MAC address "
			       << macAddr << std::endl;
    	}
    	else {
    		opp_error("Network address: %d is not from a host module with wireless nic!", netwAddr);
    	}
		return macAddr;
    }
    opp_error("Network address: %d is not from a host module!", netwAddr);
    return LAddress::L2NULL;
}

LAddress::L3Type BaseArp::myNetwAddr(const cModule* netw) const
{
    return LAddress::L3Type( netw->getId() );
}

LAddress::L2Type BaseArp::myMacAddr(const cModule *mac) const
{
    return LAddress::L2Type( mac->getParentModule()->getId() );
}
