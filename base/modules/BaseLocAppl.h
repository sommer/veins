/* -*- mode:c++ -*- ********************************************************
 * file:        BaseLocAppl.h
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
 * description: basic localization application class
 *              extend to create application for the BaseLocApplLayer
 **************************************************************************/

#ifndef BASE_LOC_APPL_H
#define BASE_LOC_APPL_H

#include <BaseModule.h>
#include "ApplPkt_m.h"
#include "LocFilter.h"

#include "BaseLocalization.h"

/**
 * @brief Base class for a localization application module
 *
 * This is the generic class for all localization application modules. 
 * If you want to implement another localization application, you have 
 * to extend this class. An example of this is TestLocAppl which can be
 * found in the folder examples/locNetwork.
 *
 * @ingroup basicModules
 *
 * @author Peterpaul Klein Haneveld
 */
class BaseLocAppl:public BaseModule {
      protected:
	/** @brief The gates of this module. */
	int lowergateIn, lowergateOut, lowerControlIn, lowerControlOut, locgateIn, locgateOut;
	/** @brief Length of the ApplPkt header. */
	int headerLength;
	BaseLocalization * loc;
      public:
	 Module_Class_Members(BaseLocAppl, BaseModule, 0);
	virtual void initialize(int);
	void handleMessage(cMessage *);
      protected:
	 virtual void handleSelfMsg(cMessage * msg) {
		EV << "BaseLocAppl: handleSelfMsg not redefined; delete msg" << endl;
		delete msg;
	};
	virtual void handleLowerMsg(cMessage * msg) {
		EV << "BaseLocAppl: handleLowerMsg not redefined; delete msg" << endl;
		delete msg;
	};
	virtual void handleLowerControl(cMessage * msg) {
		EV <<
		    "BaseLocAppl: handleLowerControl not redefined; delete msg" << endl;
		delete msg;
	};
	virtual void handleLocMsg(cMessage * msg) {
		EV << "BaseLocAppl: handleLocMsg not redefined; delete msg" << endl;
		delete msg;
	};

	void sendDown(cMessage *);
	void sendDelayedDown(cMessage *, double);
	void sendControlDown(cMessage *);
	void sendLoc(cMessage *);

	cModule * grandparentModule() const {
		return parentModule()->parentModule();
	}

	virtual const int myApplAddr() {
		return grandparentModule()->index();;
	};
	
	BaseLocalization * getLocalizationModule();
};

#endif				/* BASE_LOC_APPL_H */
