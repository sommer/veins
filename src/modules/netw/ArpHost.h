/* -*- mode:c++ -*- ********************************************************
 * file:        ArpHost.h
 *
 * author:      Jerome Rousselot
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

#ifndef ARP_HOST_H
#define ARP_HOST_H

#include "MiXiMDefs.h"
#include "ArpInterface.h"
#include "AddressingInterface.h"
#include "BaseModule.h"

/**
 * @brief A class to convert integer layer addresses
 *
 * This class assumes that a host contains a NetwLayer
 * and a Nic module. The Nic module contains a MAC address.
 * It also assumes that hosts are part of an array.
 * macaddress=netaddress= host index in array.
 *
 * @ingroup netwLayer
 *
 * @author Jerome Rousselot
 **/
class MIXIM_API ArpHost : public ArpInterface,
				public AddressingInterface,
				public BaseModule
{
    int offset;
public:
    virtual void initialize(int stage);

    /** @brief should not be called,
     *  instead direct calls to the radio methods should be used.
     */
    virtual void handleMessage( cMessage* ){
        error("ARP module cannot receive messages!");
    };

    /** @brief returns a L2 address to a given L3 address*/
    virtual LAddress::L2Type getMacAddr(const LAddress::L3Type& netwAddr) const;

    /** @brief returns a L3 address to a given L2 address*/
    virtual LAddress::L3Type getNetwAddr(const LAddress::L2Type& macAddr) const;

    /** @brief Returns the L2 address for the passed mac*/
    virtual LAddress::L2Type myMacAddr(const cModule* mac) const;

    /** @brief Returns the L3 address for the passed net*/
    virtual LAddress::L3Type myNetwAddr(const cModule* netw) const;
};

#endif
