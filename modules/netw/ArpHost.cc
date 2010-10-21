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
        hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;
	}
}

int ArpHost::getMacAddr(const int netwAddr)
{
    if(debug) {
        Enter_Method("getMacAddr(%d)",netwAddr);
    } else {
        Enter_Method_Silent();
    }
    // modification by Jerome Rousselot, CSEM
    // assumes that addresses are equal to host IDs
    // and that mac addresses == net addresses
    return netwAddr;
//    coreEV << "for host[" << simulation.getModule( netwAddr )->getParentModule()->getIndex()
//       << "]: netwAddr " << netwAddr << "; MAC address "
//       << simulation.getModule( netwAddr )->getParentModule()->getSubmodule( "nic" )->getId() <<endl;
//    return simulation.getModule(netwAddr)->getParentModule()->getSubmodule("nic")->getId();
}

int ArpHost::getNetwAddr(const int macAddr)
{
    if(coreDebug) {
        Enter_Method("getNetwAddr(%d)",macAddr);
    } else {
        Enter_Method_Silent();
    }
    // modification by Jerome Rousselot, CSEM
    // assumes that addresses are equal to host IDs
    // and that mac addresses == net addresses
    return macAddr;
//    coreEV << "for host[" << simulation.getModule( macAddr )->getParentModule()->getIndex()
//       << "]: macAddr " << macAddr << "; netw address "
//       << simulation.getModule( macAddr )->getParentModule()->getSubmodule("nic")->getId() <<endl;
//    return simulation.getModule(macAddr)->getParentModule()->getSubmodule("netw")->getId();
}

int ArpHost::myNetwAddr(cModule* netw) {
    // modification by Jerome Rousselot, CSEM
    // assumes that addresses are equal to host index.
    // and that mac addresses == net addresses
	return netw->getParentModule()->getIndex();
//    return netw->getId();
}

int ArpHost::myMacAddr(cModule *mac)
{
    // modification by Jerome Rousselot, CSEM
	// assumes that addresses are equal to host index.
    // and that mac addresses == net addresses
	return mac->getParentModule()->getParentModule()->getIndex();
//    return (mac->getParentModule())->getId();
}
