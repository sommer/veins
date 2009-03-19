/* -*- mode:c++ -*- ********************************************************
 * file:        BaseNetwLayer.h
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
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
 * description: network layer: general class for the network layer
 *              subclass to create your own network layer
 **************************************************************************/


#ifndef SIMPLE_NETW_LAYER_H
#define SIMPLE_NETW_LAYER_H

#include <BaseLayer.h>

#include "BaseArp.h"
#include "NetwPkt_m.h"
#include "SimpleAddress.h"
#include "NicControlType.h"

/**
 * @brief Base class for the network layer
 *
 * @defgroup netwLayer Network layer
 *
 * @ingroup netwLayer
 * @author Daniel Willkomm
 **/
class BaseNetwLayer : public BaseLayer
{

  protected:
    /**
     * @brief Length of the NetwPkt header
     * Read from omnetpp.ini
     **/
    int headerLength;

    /** @brief Pointer to the arp module*/
    BaseArp* arp;

    /** @brief cached variable of my networ address */
    int myNetwAddr;

public:
    //Module_Class_Members(BaseNetwLayer,BaseLayer,0);

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

  protected:
    /**
     * @name Handle Messages
     * @brief Functions to redefine by the programmer
     *
     * These are the functions provided to add own functionality to your
     * modules. These functions are called whenever a self message or a
     * data message from the upper or lower layer arrives respectively.
     *
     **/
    /*@{*/

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage* msg);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage* msg);

    /** @brief Handle self messages */
    virtual void handleSelfMsg(cMessage* msg){
	error("BaseNetwLayer does not handle self messages");
    };

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage* msg);

    /** @brief Handle control messages from lower layer */
    virtual void handleUpperControl(cMessage* msg){
        error("BaseNetwLayer does not handle control messages");
    };

    /*@}*/

    /** @brief decapsulate higher layer message from NetwPkt */
    virtual cMessage* decapsMsg(NetwPkt*);

    /** @brief Encapsulate higher layer packet into an NetwPkt*/
    virtual NetwPkt* encapsMsg(cPacket*);
};

#endif
