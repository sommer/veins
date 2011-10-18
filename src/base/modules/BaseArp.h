/* -*- mode:c++ -*- ********************************************************
 * file:        BaseArp.h
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

#ifndef BASE_ARP_H
#define BASE_ARP_H

#include "MiXiMDefs.h"
#include "ArpInterface.h"
#include "AddressingInterface.h"
#include "BaseModule.h"

/**
 * @brief A class to convert integer layer addresses
 *
 * This class takes the network layer module id as the network layer address and
 * the NIC module id as the mac layer address
 *
 * ARP implementations are divers, it does not make sense to start from a common
 * class. Their main purpose is to translate layer 2 into layer 3 addresses.
 * However, these addresses can be very different. Here, simple integers are
 * sufficient, but for hardware-in-the-loop simulations more complex ones are
 * appropriate.
 *
 * In contrast to this ARP, others may want to cache entries.
 *
 * Since this simplified ARP relies on a certain addressing scheme it implements
 * the AddressingInterface to be able to define the addresses for each network
 * and mac layer.
 *
 * @ingroup netwLayer
 * @ingroup baseModules
 *
 * @author Daniel Willkomm
 **/
class MIXIM_API BaseArp : public ArpInterface,
                          public AddressingInterface,
                          public BaseModule
{
    /** @brief Is core debugging enabled?*/
    bool coreDebug;

public:
    virtual void initialize(int stage);

    /** @brief should not be called,
     *  instead direct calls to the radio methods should be used.
     */
    virtual void handleMessage( cMessage* ){
        error("ARP module cannot receive messages!");
    };

    /** @brief returns a L2 address to a given L3 address.*/
    virtual LAddress::L2Type getMacAddr(const LAddress::L3Type& netwAddr) const;

    /** @brief Returns the L2 address for the passed mac.*/
    virtual LAddress::L2Type myMacAddr(const cModule* mac) const;

    /** @brief Returns the L3 address for the passed net.*/
    virtual LAddress::L3Type myNetwAddr(const cModule* netw) const;
};


#endif
