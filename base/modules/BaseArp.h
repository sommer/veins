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

#include <omnetpp.h>
#include <BaseModule.h>
#include <ModuleAccess.h>

/**
 * @brief A class to convert integer layer addresses
 *
 * This class takes the network layer id as the network layer
 * address and the mac layer id as the mac layer address
 *
 * ARP implementations are divers, it does not make sense to start
 * from a common class. Their main purpose is to translate layer 2
 * into layer 3 addresses. However, these addresses can be very
 * different. Here, simple integers are sufficient, but for
 * hardware-in-the-loop simulations more complex ones are appropriate.
 *
 * In contrast to this ARP, others may want to cache entries. This
 * adds another set of basic operations that may or may not make sense
 * for ARPs.
 *
 * @ingroup netwLayer
 * @author Daniel Willkomm 
 **/
class BaseArp : public BaseModule
{
    Module_Class_Members(BaseArp,BaseModule,0);

public:

    /** @brief should not be called,
     *  instead direct calls to the radio methods should be used.
     */
    virtual void handleMessage( cMessage* ){
        error("ARP module cannot receive messages!");
    };

    /** @brief returns a L2 address to a given L3 address*/
    int getMacAddr(const int netwAddr);
    
    /** @brief returns a L3 address to a given L2 address*/
    int getNetwAddr(const int macAddr);
};

class  BaseArpAccess : public ModuleAccess<BaseArp>
{
  public:
    BaseArpAccess() : ModuleAccess<BaseArp>("arp") {}
};


#endif
