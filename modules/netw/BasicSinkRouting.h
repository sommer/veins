/* -*- mode:c++ -*- ********************************************************
 * file:        BasicSinkRouting.h
 *
 * author:      Tom Parker
 *
 * copyright:   (C) 2006 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     wsn-specific modules
 * description: network layer: basic source-to-sink routing
 ***************************************************************************/

#ifndef BASIC_SINK_LAYER_H
#define BASIC_SINK_LAYER_H

#include "QueuedRouting.h"

#include "BaseArp.h"
#include "NetwPkt_m.h"
#include "SinkInfo_m.h"
#include "Timer.h"

/**
 * @brief Basic source-to-sink routing
 * 
 * @ingroup netwLayer
 * @author Tom Parker
 **/
class BasicSinkRouting : public QueuedRouting, public Timer
{

protected:
	typedef enum {SINK_BCAST=1, UPPER_TYPE} NetworkTypes;
    
	NetwPkt *buildSink(SinkInfo *sink=NULL, int from=-1);

	std::map<int,SinkInfo*> *sinks;

	bool isSink;

	void printSinks();
	bool setNextHop(NetwPkt *pkt);
 
public:
	//Module_Class_Members(BasicSinkRouting,QueuedRouting,0);

	/** @brief Initialization of the module and some variables*/
	virtual void initialize(int stage);

	virtual void finish();

	virtual ~BasicSinkRouting();
	virtual int upperKind() {return UPPER_TYPE;}
    
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

	/*@}*/

	/** @brief Send packet to network */
	virtual void toNetwork(NetwPkt *out);

	virtual void handleTimer(unsigned int count);

	/** @brief Provide MAC address for 'special' network addresses (see SpecialAddresses.h) */
	virtual int specialMACAddress(int netwAddr);

	/** @brief Provide network address to use for 'special' network addresses (see SpecialAddresses.h) */
	virtual int specialNetwAddress(int netwAddr);
};

#endif
