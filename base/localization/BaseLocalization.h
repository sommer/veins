/* -*- mode:c++ -*- ********************************************************
 * file:        BaseLocalization.h
 *
 * author:      Peterpaul Klein Haneveld
 *
 * copyright:   (C) 2007 Parallel and Distributed Systems Group (PDS) at
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
 * part of:     mixim framework
 * description: localization layer: general class for the network layer
 *              subclass to create your own localization layer
 **************************************************************************/


#ifndef BASELOC_H
#define BASELOC_H

#include <BaseLayer.h>

#include "BaseArp.h"
#include "LocPkt_m.h"

/**
 * @brief Base class for the localization layer
 * 
 * @ingroup localization
 * @author Peterpaul Klein Haneveld
 **/
class BaseLocalization:public BaseLayer {

      protected:
	/**
	 * @brief Length of the LocPkt header 
	 * Read from omnetpp.ini 
	 **/
	int headerLength;

      public:
	 Module_Class_Members(BaseLocalization, BaseLayer, 0);

	/** @brief Initialization of the module and some variables*/
	virtual void initialize(int);
	Coord getPosition();

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

	/** @brief Handle self messages */
	virtual void handleSelfMsg(cMessage * msg) {
		error("BaseLocalization does not handle self messages");
	};

	/** @brief Handle control messages from lower layer */
	virtual void handleLowerControl(cMessage * msg);

	/** @brief Handle control messages from lower layer */
	virtual void handleUpperControl(cMessage * msg) {
		error("BaseLocalization does not handle control messages");
	};

	/*@} */

	/** @brief decapsulate higher layer message from LocPkt */
	virtual cMessage *decapsMsg(LocPkt *);

	/** @brief Encapsulate higher layer packet into an LocPkt*/
	virtual LocPkt *encapsMsg(cMessage *);
};

#endif
