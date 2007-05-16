/* -*- mode:c++ -*- ********************************************************
 * file:        BasicSinkRouting.h
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


#ifndef BASIC_SINK_LAYER_H
#define BASIC_SINK_LAYER_H

#include <BaseNetwLayer.h>
#include <queue>

#include "BaseArp.h"
#include "NetwPkt_m.h"
#include "SinkAddress.h"
#include "SinkInfo_m.h"

/**
 * @brief Base class for the network layer
 * 
 * @ingroup netwLayer
 * @author Daniel Willkomm
 **/
class BasicSinkRouting : public BaseNetwLayer
{

  protected:
    /**
     * @brief Length of the NetwPkt header 
     * Read from omnetpp.ini 
     **/
    int headerLength;
    
    /** @brief Pointer to the arp module*/
    BaseArp* arp;

	typedef enum {SINK_BCAST=1, UPPER_TYPE} NetworkTypes;
    
	NetwPkt *buildPkt(int kind, int netwAddr,const char* name);
	NetwPkt *buildSink(SinkInfo *sink=NULL, int from=-1);

    /** @brief cached variable of my network address */
    int myNetwAddr;

	std::queue<NetwPkt*> *msgQueue;

	std::map<int,SinkInfo*> *sinks;

	bool msgBusy;
	bool isSink;

	/** Number of timers. Initialised to 0. Set by @b init_timers() */
	unsigned int timer_count;
	/** Timer message array */
	cMessage *timers;

	void printSinks();
	bool setNextHop(NetwPkt *pkt);
	void sendQueued();
 
public:
    Module_Class_Members(BasicSinkRouting,BaseNetwLayer,0);

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int stage);

	virtual void finish();

	virtual ~BasicSinkRouting();
    
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
    virtual void handleSelfMsg(cMessage* msg)
	{
		handleTimer(msg->kind());
	}

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage* msg);

    /** @brief Handle control messages from upper layer */
    virtual void handleUpperControl(cMessage* msg){
        error("BasicSinkRouting does not handle control messages");
    };

    /*@}*/

    /** @brief decapsulate higher layer message from NetwPkt */
    virtual cMessage* decapsMsg(NetwPkt*);  

    /** @brief Encapsulate higher layer packet into an NetwPkt*/
    virtual NetwPkt* encapsMsg(cMessage*);  

	/** @brief Send packet to network */
	void toNetwork(NetwPkt *out);

	/* Begin Timers */

	/** Initialise a set of timers for this protocol layer
		@param count Number of timers used by this layer
	 */	
	void initTimers(unsigned int count);

	/** Set one of the timers to fire at a point in the future.
		If the timer has already been set then this discards the old information.
		Must call @b initTimers() before using.
		@param index Timer number to set. Must be between 0 and the value given to @b initTimers()
		@param when Time in seconds in the future to fire the timer
	 */
	void setTimer(unsigned int index, double when);

	/** Cancel an existing timer set by @b setTimer()
		If the timer has not been set, or has already fires, this does nothing
		Must call @b initTimers() before using.
		@param index Timer to cancel. Must be between 0 and the value given to @b initTimers()
	 */
	void cancelTimer(unsigned int index);

	/** Fires on expiration of a timer.
		Fires after a call to @b setTimer(). Subclasses should override this.
		@param index Timer number that fired. Will be between 0 and the value given to @b initTimers()
	*/	

	float remainingTimer(unsigned int index);

	virtual void handleTimer(unsigned int count);
	/* End Timers */

};

#endif
