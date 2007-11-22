/* -*- mode:c++ -*- ********************************************************
 * file:        QueuedRouting.h
 *
 * author:      Tom Parker
 *
 * copyright:   (C) 2006 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; version 
 *              2 of the License.
 *
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     routing modules
 * description: network layer: basic routing with queues
 ***************************************************************************/

#ifndef QUEUED_ROUTINGLAYER_H
#define QUEUED_ROUTINGLAYER_H

#include <BaseNetwLayer.h>
#include <queue>

#include "NetwPkt_m.h"
#include "SpecialAddresses.h"

/**
 * @brief Basic routing with queues
 * 
 * @ingroup netwLayer
 * @author Tom Parker
 **/
class QueuedRouting : public BaseNetwLayer
{

protected:
	virtual NetwPkt *buildPkt(int kind, int netwAddr,const char* name);

	std::queue<NetwPkt*> *msgQueue;

	bool msgBusy;

	void sendQueued();
 
public:
	Module_Class_Members(QueuedRouting,BaseNetwLayer,0);
	
	/** @brief Initialization of the module and some variables*/
	virtual void initialize(int stage);

	virtual void finish();

	virtual ~QueuedRouting();
    
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
	
	virtual NetwPkt* encapsMsg(cMessage * msg);

	/** @brief Handle messages from upper layer */
	virtual void handleUpperMsg(cMessage* msg);
    
	/** @brief Handle control messages from lower layer */
	virtual void handleLowerControl(cMessage* msg);

	/*@}*/

	/** @brief Returns the 'kind' that messages from upper layers should have */
	virtual int upperKind() {return 0;}
    
	/** @brief Send packet to network */
	virtual void toNetwork(NetwPkt *out);

	/** @brief Provide MAC address for 'special' network addresses (see SpecialAddresses.h) */
	virtual int specialMACAddress(int netwAddr) {opp_error("QueuedRouting doesn't handle special network addresses");return -1;}
	/** @brief Provide network address to use for 'special' network addresses (see SpecialAddresses.h) */
	virtual int specialNetwAddress(int netwAddr) {opp_error("QueuedRouting doesn't handle special network addresses");return -1;}
};

#endif
