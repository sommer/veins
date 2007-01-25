/* -*- mode:c++ -*- ********************************************************
 * file:        LocFilter.h
 *
 * author:      Peterpaul Klein Haneveld
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
 * description: this class implements filtering for BaseLocApplLayer
 **************************************************************************/

#ifndef LOC_FILTER_H
#define LOC_FILTER_H

#include <BaseModule.h>

/**
 * @brief Filter module for the BaseLocApplLayer.
 *
 * This module forwards messages from the BaseLocAppl and 
 * BaseLocalization module to the network layer, and vice
 * versa.
 *
 * @ingroup basicModules
 *
 * @author Peterpaul Klein Haneveld
 */
class LocFilter:public BaseModule {
      protected:
	int lowergateIn, lowergateOut, lowerControlIn, lowerControlOut,
	    applgateIn, applgateOut, applControlIn, applControlOut,
	    locgateIn, locgateOut, locControlIn, locControlOut;
	int headerLength;
      public:
	 Module_Class_Members(LocFilter, BaseModule, 0);
	virtual void initialize(int);
	void handleMessage(cMessage *);
      protected:
	void handleSelfMsg(cMessage * msg);
	void handleLowerMsg(cMessage * msg);
	void handleLowerControl(cMessage * msg);
	void handleApplMsg(cMessage * msg);
	void handleApplControl(cMessage * msg);
	void handleLocMsg(cMessage * msg);
	void handleLocControl(cMessage * msg);
};

#endif				/* LOC_FILTER_H */
