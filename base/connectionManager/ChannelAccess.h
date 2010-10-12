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

#include "BatteryAccess.h"
#include "Move.h"
#include "BaseWorldUtility.h"
#include "AirFrame_m.h"

#include "BaseConnectionManager.h"

/**
 * @brief Basic class for all physical layers, please don't touch!!
 *
 * This class is not supposed to work on its own, but it contains
 * functions and lists that cooperate with ConnectionManager to handle
 * the dynamically created gates. This means EVERY physical layer (the lowest
 * layer in a host) has to be derived from this class!!!!
 *
 * Please don't touch this class.
 *
 * @author Marc Loebbers
 * @ingroup connectionManager
 * @ingroup phyLayer
 * @ingroup baseModules
 **/
class ChannelAccess : public BatteryAccess
{
protected:
    /** @brief use sendDirect or not?*/
    bool useSendDirect;

    /** @brief Pointer to the PropagationModel module*/
    BaseConnectionManager* cc;

    /** @brief debug this core module? */
    bool coreDebug;

	/** @brief Defines if the physical layer should simulate propagation delay.*/
	bool usePropagationDelay;

    /** @brief Last move of this host */
    Move move;
    /** @brief category number given by bb for RSSI */
    int catMove;

    /** @brief Is this module already registered with ConnectionManager? */
    bool isRegistered;

    /** @brief Pointer to the World Utility, to obtain some global information*/
	BaseWorldUtility* world;

protected:
	/**
	 * @brief Calculates the propagation delay to the passed receiving nic.
	 */
	simtime_t calculatePropagationDelay(const NicEntry* nic);

	/** @brief Sends a message to all nics connected to this one.
	 *
	 * This function has to be called whenever a packet is supposed to be
	 * sent to the channel. Don't try to figure out what gates you have
	 * and which ones are connected, this function does this for you!
	 *
	 * depending on which ConnectionManager module is used, the messages are
	 * send via sendDirect() or to the respective gates.
	 **/
	void sendToChannel(cPacket *msg);

public:
	/**
	 * @brief Returns a pointer to the ConnectionManager responsible for the
	 * passed NIC module.
	 *
	 * @param nic a pointer to a NIC module
	 * @return a pointer to a connection manager module or NULL if an error
	 * occurred
	 */
	static BaseConnectionManager* getConnectionManager(cModule* nic);

    /** @brief Register with ConnectionManager and subscribe to hostPos
     *
	 * Upon initialization ChannelAccess registers the nic parent module
	 * to have all its connections handeled by ConnectionManager
	 **/
    virtual void initialize(int stage);

    /**
     * @brief Called by Blackboard to inform of changes
     *
	 * ChannelAccess is subscribed to position changes and informs the
	 * ConnectionManager
	 */
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);
};

#endif

