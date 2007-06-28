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

#include <BaseNetwLayer.h>
#include <queue>

#include "BaseArp.h"
#include "NetwPkt_m.h"
#include "SinkAddress.h"
#include "SinkInfo_m.h"
#include "Timer.h"

/**
 * @brief Basic source-to-sink routing
 * 
 * @ingroup netwLayer
 * @author Tom Parker
 **/
class BasicSinkRouting : public BaseNetwLayer, public Timer
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

	virtual void handleTimer(unsigned int count);

};

#endif
