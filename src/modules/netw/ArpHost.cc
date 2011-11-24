/* -*- mode:c++ -*- ********************************************************
 * file:        ArpHost.cc
 *
 * author:      Daniel Willkomm, Jerome Rousselot
 *
 * copyright:   (C) 2010 CSEM SA, Neuchatel, Switzerland.
 * 				(C) 2005 Telecommunication Networks Group (TKN) at
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

#include "ArpHost.h"

Define_Module(ArpHost);

void ArpHost::initialize(int stage) {
	BaseModule::initialize(stage);
    if(stage==0) {
        offset = par("offset");
	}
}

LAddress::L2Type ArpHost::getMacAddr(const LAddress::L3Type& netwAddr) const
{
    Enter_Method_Silent();
    // modification by Jerome Rousselot, CSEM
    // assumes that addresses are equal to host IDs
    // and that mac addresses == net addresses
    return LAddress::L2Type(netwAddr);
//    debugEV << "for host[" << simulation.getModule( netwAddr )->getParentModule()->getIndex()
//       << "]: netwAddr " << netwAddr << "; MAC address "
//       << simulation.getModule( netwAddr )->getParentModule()->getSubmodule( "nic" )->getId() <<endl;
//    return simulation.getModule(netwAddr)->getParentModule()->getSubmodule("nic")->getId();
}

LAddress::L3Type ArpHost::getNetwAddr(const LAddress::L2Type& macAddr) const
{
    Enter_Method_Silent();
    // modification by Jerome Rousselot, CSEM
    // assumes that addresses are equal to host IDs
    // and that mac addresses == net addresses
#ifdef MIXIM_INET
    return LAddress::L3Type(macAddr.getInt());
#else
    return LAddress::L3Type(macAddr);
#endif
//    debugEV << "for host[" << simulation.getModule( macAddr )->getParentModule()->getIndex()
//       << "]: macAddr " << macAddr << "; netw address "
//       << simulation.getModule( macAddr )->getParentModule()->getSubmodule("nic")->getId() <<endl;
//    return simulation.getModule(macAddr)->getParentModule()->getSubmodule("netw")->getId();
}

LAddress::L3Type ArpHost::myNetwAddr(const cModule* netw) const
{
    // modification by Jerome Rousselot, CSEM
    // assumes that addresses are equal to host index.
    // and that mac addresses == net addresses
	return LAddress::L3Type(netw->getParentModule()->getIndex()+offset);
//    return netw->getId();
}

LAddress::L2Type ArpHost::myMacAddr(const cModule *mac) const
{
    // modification by Jerome Rousselot, CSEM
	// assumes that addresses are equal to host index.
    // and that mac addresses == net addresses
	return LAddress::L2Type(mac->getParentModule()->getParentModule()->getIndex()+offset);
//    return (mac->getParentModule())->getId();
}
