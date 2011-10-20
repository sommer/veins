/* -*- mode:c++ -*- *******************************************************
 * file:        NetwControlInfo.h
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
 * description: - control info to pass the netw addresses between the
 *                network and application layer
 **************************************************************************/

#ifndef APPLCONTROLINFO_H
#define APPLCONTROLINFO_H

#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "SimpleAddress.h"

/**
 * @brief Control info netw messages
 *
 * Control Info to pass interface information from the network to the
 * application layer and vice versa. The application layer passes the
 * destination netw address to the network layer, whereas the network
 * layer uses the control info to pass the source netw address to the
 * application layer
 *
 * @ingroup utils
 * @ingroup baseUtils
 * @ingroup netwLayer
 * @ingroup applLayer
 * @author Daniel Willkomm
 **/
class MIXIM_API NetwControlInfo : public cObject
{
  protected:
    /** @brief netw address of the sending or receiving node*/
    LAddress::L3Type netwAddr;

  public:
    /** @brief Default constructor*/
    NetwControlInfo(const LAddress::L3Type& addr = LAddress::L3NULL) : netwAddr(addr) {};
    /** @brief Destructor*/
    virtual ~NetwControlInfo(){};

    /** @brief Getter method*/
    virtual const LAddress::L3Type& getNetwAddr(){
        return netwAddr;
    };

    /** @brief Setter method*/
    virtual void setNetwAddr(const LAddress::L3Type& addr){
        netwAddr = addr;
    };


    /**
     * @brief Attaches a "control info" structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the destination MAC address.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg	The message where the "control info" shall be attached.
     * @param pAddr	The network address of to save.
     */
    static cObject *const setControlInfo(cMessage *const pMsg, const LAddress::L3Type& pAddr) {
    	NetwControlInfo *const cCtrlInfo = new NetwControlInfo(pAddr);
    	pMsg->setControlInfo(cCtrlInfo);

    	return cCtrlInfo;
    }
    /**
     * @brief extracts the address from "control info".
     */
    static const LAddress::L3Type& getAddressFromControlInfo(cObject *const pCtrlInfo) {
    	NetwControlInfo *const cCtrlInfo = dynamic_cast<NetwControlInfo *const>(pCtrlInfo);

    	if (cCtrlInfo)
    		return cCtrlInfo->getNetwAddr();
    	return LAddress::L3NULL;
    }
};


#endif
