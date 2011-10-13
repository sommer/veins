/* -*- mode:c++ -*- *******************************************************
 * file:        NetwToMacControlInfo.h
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
 **************************************************************************
 * part of:     framework implementation developed by tkn
 * description: - control info to pass next hop to the MAC layer
 **************************************************************************/

#ifndef NETWTOMACCONTROLINFO_H
#define NETWTOMACCONTROLINFO_H

#include <omnetpp.h>

/**
 * @brief Control info to pass next hop L2 addr from netw to MAC layer
 *
 * Control Info to pass interface information from the network to the
 * MAC layer Currently the only information necessary is the MAC
 * address of the next hop, which has to be determined by ARP or some
 * similar mechanism
 *
 * @ingroup baseUtils
 * @ingroup macLayer
 * @ingroup netwLayer
 * @author Daniel Willkomm
 **/
class NetwToMacControlInfo : public cObject
{
  protected:
    /** @brief MAC address of the sending or receiving node*/
    int nextHopMac;


  public:
    /** @brief Default constructor*/
    NetwToMacControlInfo(const int addr) : nextHopMac(addr) {};

    /** @brief Destructor*/
    virtual ~NetwToMacControlInfo() {};

    /** @brief Getter method */
    virtual const int getNextHopMac() {
    	return nextHopMac;
    };

    /** @brief Setter method */
    virtual void setNextHopMac(const int addr){
    	nextHopMac = addr;
    };
};


#endif
