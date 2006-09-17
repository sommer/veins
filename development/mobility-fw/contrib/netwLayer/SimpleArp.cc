/* -*- mode:c++ -*- ********************************************************
 * file:        SimpleArp.cc
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

#include "SimpleArp.h"

Define_Module(SimpleArp);

int SimpleArp::getMacAddr(const int netwAddr)
{
    if(debug) {
        Enter_Method("getMacAddr(%d)",netwAddr);
    } else {
        Enter_Method_Silent();
    }
    EV << "for host[" << simulation.module( netwAddr )->parentModule()->index()
       << "]: netwAddr " << netwAddr << "; MAC address " 
       << simulation.module( netwAddr )->parentModule()->submodule( "nic" )->id() <<endl;
    return simulation.module(netwAddr)->parentModule()->submodule("nic")->id();
}

int SimpleArp::getNetwAddr(const int macAddr)
{
    if(debug) {
        Enter_Method("getNetwAddr(%d)",macAddr);
    } else {
        Enter_Method_Silent();
    }
    EV << "for host[" << simulation.module( macAddr )->parentModule()->index()
       << "]: macAddr " << macAddr << "; netw address " 
       << simulation.module( macAddr )->parentModule()->submodule("nic")->id() <<endl;
    return simulation.module(macAddr)->parentModule()->submodule("netw")->id();
}
