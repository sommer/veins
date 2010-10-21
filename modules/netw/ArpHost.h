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

#include <ArpInterface.h>
#include <AddressingInterface.h>
#include <BaseModule.h>
#include <FindModule.h>

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
class ArpHost : public ArpInterface,
				public AddressingInterface,
				public BaseModule
{
    //Module_Class_Members(BaseArp,BaseModule,0);
	bool coreDebug;

public:
	virtual void initialize(int stage);

    /** @brief should not be called,
     *  instead direct calls to the radio methods should be used.
     */
    virtual void handleMessage( cMessage* ){
        error("ARP module cannot receive messages!");
    };

    /** @brief returns a L2 address to a given L3 address*/
    virtual int getMacAddr(const int netwAddr);

    /** @brief returns a L3 address to a given L2 address*/
    virtual int getNetwAddr(const int macAddr);

    /** @brief Returns the L2 address for the passed mac*/
    virtual int myMacAddr(cModule* mac);

    /** @brief Returns the L3 address for the passed net*/
    virtual int myNetwAddr(cModule* netw);
};

#endif
