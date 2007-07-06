/* -*- mode:c++ -*- ********************************************************
 * file:        BaseAggLayer.h
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
 * description: aggregation layer: basic core. subclass to build your own
 *              aggregation protocol
 ***************************************************************************/


#ifndef BASE_AGG_LAYER_H
#define BASE_AGG_LAYER_H

#include <BaseLayer.h>

#include "BaseArp.h"
#include "AggPkt_m.h"
#include "SimpleAddress.h"

#define DBG(...) {char *dbg_out;asprintf(&dbg_out,## __VA_ARGS__);EV<<dbg_out;free(dbg_out);if (isnan(simTime())){throw new cRuntimeError("nan simtime");}}
#define DBG_clear(...) {char *dbg_out;asprintf(&dbg_out,## __VA_ARGS__);EV_clear<<dbg_out;free(dbg_out);}

/**
 * @brief Base class for the aggregation layer
 * 
 * @ingroup AggLayer
 * @author Tom Parker
 **/
class BaseAggLayer:public BaseLayer
{

  protected:
	/**
     * @brief Length of the AggPkt header 
     * Read from omnetpp.ini 
     **/
	int headerLength;

	/** @brief Pointer to the arp module*/
	BaseArp *arp;

	/** @brief cached variable of my Aggor address */
	int myAggAddr;

	bool isSink;

  public:
	 Module_Class_Members(BaseAggLayer, BaseLayer, 0);

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
	/*@{ */

	/** @brief Handle messages from upper layer */
	 virtual void handleUpperMsg(cMessage * msg);

	/** @brief Handle messages from lower layer */
	virtual void handleLowerMsg(cMessage * msg);

	/** @brief Handle control messages from lower layer */
	virtual void handleLowerControl(cMessage * msg)
	{
		error("BaseAggLayer does not handle control messages");
	};

	/** @brief Handle control messages from lower layer */
	virtual void handleUpperControl(cMessage * msg)
	{
		error("BaseAggLayer does not handle control messages");
	};

	virtual void handleSelfMsg(cMessage * msg)
	{
		error("BaseAggLayer doesn't have it's own self messages");
	}
	/*@} */

	/** @brief decapsulate higher layer message from AggPkt */
	//virtual cMessage *decapsMsg(AggPkt *);

	/** @brief Encapsulate higher layer packet into an AggPkt*/
	//virtual AggPkt *encapsMsg(cMessage *);



};

#endif
