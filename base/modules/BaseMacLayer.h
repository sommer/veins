/* -*- mode:c++ -*- ********************************************************
 * file:        BaseMacLayer.h
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


#ifndef BASE_MAC_LAYER_H
#define BASE_MAC_LAYER_H

#include <omnetpp.h>
#include "BaseLayer.h"
#include "MacPkt_m.h"
#include <MacToPhyInterface.h>

/**
 * @brief A very simple MAC module template which provides de- and
 * encapsulation of messages using the standard addresses and
 * MacControlInfo of MiXiM. It also provides basic handling of lower
 * layer messages.
 *
 * @ingroup macLayer
 * @ingroup basicModules
 * @author Daniel Willkomm, Karl Wessel
 */
class BaseMacLayer : public BaseLayer
{
protected:

	/** @brief Handler to the physical layer.*/
	MacToPhyInterface* phy;

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
    //Module_Class_Members( BaseMacLayer, BaseLayer, 0 );

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

    /**
     * @brief Handle messages comming from the network layer
     *
     * Here is the place to implement a real medium access functionality.
     *
     * If message arrives from upper layer, make a MAC packet from it
     * (encapsMsg) and send it down (sendDown).
     *
     *  @sa encapsMsg, sendDown
     */
    virtual void handleUpperMsg(cMessage *msg);

    /**
     * If message arrives from lower layer, check whether it is for
     * us. Send it up if yes.
     */
    virtual void handleLowerMsg(cMessage *msg);

    virtual void handleSelfMsg(cMessage* msg){
	error("BaseMacLayer does not handle self messages");
    };
    virtual void handleLowerControl(cMessage* msg);

    virtual void handleUpperControl(cMessage* msg){
	error("BaseMacLayer does not handle control messages from upper layers");
    };

protected:
    /** @brief decapsulate the network message from the MacPkt */
    virtual cPacket* decapsMsg(MacPkt*);

    /** @brief Encapsulate the NetwPkt into an MacPkt */
    virtual MacPkt* encapsMsg(cPacket*);

    /**
     * @brief Creates a simple Signal defined over time with the
     * passed parameters.
     *
     * Convenience method to be able to create the appropriate
     * Signal for the MacToPhyControlInfo without needing to care
     * about creating Mappings.
     */
    virtual Signal* createSignal(simtime_t start, simtime_t length, double power, double bitrate);

    /**
     * @brief Creates a simple Mapping with a constant curve
     * progression at the passed value.
     *
     * Used by "createSignal" to create the power and bitrate mapping.
     */
    Mapping* createConstantMapping(simtime_t start, simtime_t end, double value);
};

#endif
