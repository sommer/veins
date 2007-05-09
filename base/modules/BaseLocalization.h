/* -*- mode:c++ -*- ********************************************************
 * file:        BaseLocalization.h
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
 * description: basic localization class
 *              extend to implement a localization algorithm
 **************************************************************************/

#ifndef BASE_LOCALIZATION_H
#define BASE_LOCALIZATION_H

#include <BaseModule.h>
#include "ApplPkt_m.h"
#include "LocFilter.h"

/**
 * @brief Base class for a localization module
 *
 * This is the generic class for all localization modules. If you want to 
 * implement another localization module / algorithm, you have to extend
 * this class. An example of this is TestLocalization which can be found
 * in the folder examples/locNetwork.
 *
 * @ingroup basicModules
 *
 * @author Peterpaul Klein Haneveld
 */
class BaseLocalization:public BaseModule {
protected:
	/** @brief The gates of this module. */
	int lowergateIn, lowergateOut, lowerControlIn, lowerControlOut, applgateIn, applgateOut;
	/** @brief Length of the ApplPkt header. */
	int headerLength;
	/** @brief Specifies whether this node is an anchor node. */
	bool isAnchor;
public:
	Module_Class_Members(BaseLocalization, BaseModule, 0);
	virtual void initialize(int);
	void handleMessage(cMessage *);
protected:
	virtual void handleSelfMsg(cMessage * msg) {
		EV << "BaseLocalization: handleSelfMsg not redefined; delete msg" << endl;
		delete msg;
	};
	virtual void handleLowerMsg(cMessage * msg) {
		EV << "BaseLocalization: handleLowerMsg not redefined; delete msg" << endl;
		delete msg;
	};
	virtual void handleLowerControl(cMessage * msg) {
		EV << "BaseLocalization: handleLowerControl not redefined; delete msg" << endl;
		delete msg;
	};
	virtual void handleApplMsg(cMessage * msg) {
		EV << "BaseLocalization: handleApplMsg not redefined; delete msg" << endl;
		delete msg;
	};

	void sendDown(cMessage *);
	void sendDelayedDown(cMessage *, double);
	void sendControlDown(cMessage *);
	void sendAppl(cMessage *);

	cModule * grandparentModule() const {
		return parentModule()->parentModule();
	}

	virtual const int myApplAddr() {
		return grandparentModule()->index();
	}
};

#endif				/* BASE_LOCALIZATION_H */
