/* -*- mode:c++ -*- ********************************************************
 * file:        BasicMacLayer.h
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
 * description: basic MAC layer class
 *              subclass to create your own MAC layer
 **************************************************************************/


#ifndef BASICMAC_LAYER_H
#define BASICMAC_LAYER_H

#include <omnetpp.h>
#include "BasicLayer.h"
#include "MacPkt_m.h"

/**
 * @brief A very simple MAC module template which provides de- and
 * encapsulation of messages using the standard addresses and
 * MacControlInfo of the MF. It also provides basic handling of lower
 * layer messages.
 *
 * @ingroup macLayer
 * @ingroup basicModules
 * @author Daniel Willkomm
 */
class BasicMacLayer : public BasicLayer
{
protected:
    
    /**
     * @brief Length of the MacPkt header 
     **/
    int headerLength;

    /**
     * @brief MAC address (simply module id)
     **/
    int myMacAddr;
    
    /** @brief debug this core module? */
    bool coreDebug;

public:
    Module_Class_Members( BasicMacLayer, BasicLayer, 0 );

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

    /**
     * @brief Handle messages comming from the network layer
     *
     * Here is the place to implement a real medium access functionality.
     *
     * If message arrives from upper layer, make a MAC packet from it
     * (encapsMsg) and send it down (sendDOwn).
     *
     *  @sa encapsMsg, sendDown
     */
    virtual void handleUpperMsg(cMessage *msg) = 0;
    
    /**
     * If message arrives from lower layer, check whether it is for
     * us. Send it up if yes.
     */
    virtual void handleLowerMsg(cMessage *msg);

protected:
    /** @brief decapsulate the network message from the MacPkt */
    virtual cMessage* decapsMsg(MacPkt*);

    /** @brief Encapsulate the NetwPkt into an MacPkt */
    virtual MacPkt* encapsMsg(cMessage*);
};

#endif
