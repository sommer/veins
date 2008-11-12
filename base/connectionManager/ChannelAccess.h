/* -*- mode:c++ -*- ********************************************************
 * file:        ChannelAccess.h
 *
 * author:      Marc Loebbers
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
 * description: - Base class for physical layers
 *              - if you create your own physical layer, please subclass
 *                from this class and use the sendToChannel() function!!
 **************************************************************************/


#ifndef CHANNEL_ACCESS_H
#define CHANNEL_ACCESS_H

#include <omnetpp.h>
#include <vector>

#include "BaseModule.h"
#include "Move.h"
#include "BaseWorldUtility.h"
#include "AirFrame_m.h"

#include "BaseConnectionManager.h"

/**
 * @brief Basic class for all physical layers, please don't touch!!
 *
 * This class is not supposed to work on its own, but it contains
 * functions and lists that cooperate with ConnectionManager to handle
 * the dynamically created gates. This means EVERY SnrEval (the lowest
 * layer in a host) has to be derived from this class!!!! And please
 * follow the instructions on how to declare a physical layer in a
 * .ned file in "The Design of a Mobility Framework in OMNeT++"
 * paper.
 *
 * Please don't touch this class.
 *
 * @author Marc Loebbers
 * @ingroup connectionManager
 * @ingroup phyLayer
 * @ingroup basicModules
 **/
class ChannelAccess : public BaseModule
{
    //Module_Class_Members( ChannelAccess, BaseModule, 0 );

protected:
    /** @brief use sendDirect or not?*/
    bool useSendDirect;

    /** @brief Pointer to the PropagationModel module*/
    BaseConnectionManager* cc;

    /** @brief debug this core module? */
    bool coreDebug;

    /** @brief Sends a message to all nics connected to this one.*/
    void sendToChannel(cPacket *msg);

	/** Defines if the physical layer should simulate propagation delay.*/
	bool usePropagationDelay;

    /**
	 * Calculates the propagation delay to the passed receiving nic.
	 */
	simtime_t calculatePropagationDelay(const NicEntry* nic);

    /** @brief Last move of this host */
    Move move;
    /** @brief category number given by bb for RSSI */
    int catMove;

    /**
     * @brief Is this module already registered with ConnectionManager?
     */
    bool isRegistered;

    /* Pointer to the World Utility, to obtain some global information*/
	BaseWorldUtility* world;

public:
    /** @brief Register with ConnectionManager and subscribe to hostPos*/
    virtual void initialize(int stage);

    /**
     * called by Blackboard to inform of changes
     */
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);
};

#endif

