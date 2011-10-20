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
    cModule* netwLayer = simulation.getModule( static_cast<long>(netwAddr) );
    if(!netwLayer) {
    	opp_error("Invalid network address: %d! Could not find a module with "
				  "that id.", netwAddr);
    }
    LAddress::L2Type macAddr( netwLayer->getParentModule()->getSubmodule( "nic" )->getId() );
    coreEV << "for host[" << netwLayer->getParentModule()->getIndex()
       << "]: netwAddr " << netwAddr << "; MAC address "
       << macAddr << std::endl;
    return macAddr;
}

/*
int BaseArp::getNetwAddr(const int macAddr)
{
    Enter_Method("getNetwAddr(%d)",macAddr);
    coreEV << "for host[" << simulation.getModule( macAddr )->getParentModule()->getIndex()
       << "]: macAddr " << macAddr << "; netw address "
       << simulation.getModule( macAddr )->getParentModule()->getSubmodule("nic")->getId() <<endl;
    return simulation.getModule(macAddr)->getParentModule()->getSubmodule("netw")->getId();
}
*/

LAddress::L3Type BaseArp::myNetwAddr(const cModule* netw) const
{
    return LAddress::L3Type( netw->getId() );
}

LAddress::L2Type BaseArp::myMacAddr(const cModule *mac) const
{
    return LAddress::L2Type( mac->getParentModule()->getId() );
}
