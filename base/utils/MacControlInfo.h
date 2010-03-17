/* -*- mode:c++ -*- *******************************************************
 * file:        MacControlInfo.h
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

#ifndef MACCONTROLINFO_H
#define MACCONTROLINFO_H

#include <omnetpp.h>

// TODO separate into two classes (one for each direction),
// class MacToUpperControlInfo has corresponding TODO-comment
// TODO update documentation
/**
 * @brief Control info to pass next hop L2 addr from netw to MAC layer
 *
 * Control Info to pass interface information from the network to the
 * MAC layer Currently the only information necessary is the MAC
 * address of the next hop, which has to be determined by ARP or some
 * similar mechanism
 *
 *
 * @ingroup baseUtils
 * @ingroup macLayer
 * @ingroup netwLayer
 * @ingroup utils
 * @author Daniel Willkomm
 **/
class MacControlInfo : public cObject
{
  protected:
    /** @brief MAC address of the sending or receiving node*/
    int nextHopMac;

    /** @brief bit-error rate */
    double ber;
    double rssi;  // CSEM Jérôme Rousselot -- allows the MAC layer to forward RSSI information to the routing layer

  public:
    /** @brief Default constructor*/
    MacControlInfo(const int addr) : nextHopMac(addr), ber(0), rssi(0) {};

    /** @brief Destructor*/
    virtual ~MacControlInfo(){};

    /** @brief Getter method */
    virtual const int getNextHopMac() {
	return nextHopMac;
    };

    /** @brief Setter method */
    virtual void setNextHopMac(const int addr){
	nextHopMac = addr;
    };

    /** @brief Getter for bit-error rate */
    virtual const double getBER() {
      return ber;
    }

    /** @brief Setter for bit-error rate */
    void setBER(double _ber) {
    	ber = _ber;
    }

    virtual const double getRSSI() {
    	return rssi;
    }

    void setRSSI(double _rssi) {
    	rssi = _rssi;
    }
};


#endif
