/* -*- mode:c++ -*- ********************************************************
 * file:        TestLocAppl.h
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
 * description: basic localization application class
 *              extend to create application for the BaseLocApplLayer
 **************************************************************************/

/* ************************************************************************
 * Peterpaul Klein Haneveld:
 **************************************************************************
 * This file is essentially a copy of TestApplLayer with just the following
 * replacements:
 *
 * TestApplLayer        -> TestLocAppl
 * BaseApplLayer        -> BaseLocAppl
 * TEST_APPL_LAYER_H    -> TEST_LOC_APPL_H
 **************************************************************************/

#ifndef TEST_LOC_APPL_H
#define TEST_LOC_APPL_H

#include "BaseLocAppl.h"


/**
 * @brief Test class for the application layer
 * 
 * In this implementation all nodes randomly send broadcast packets to
 * all connected neighbors. Every node that receives this packet will
 * send a reply to the originator node.
 *
 * @ingroup applLayer
 * @author Daniel Willkomm
 **/
class TestLocAppl:public BaseLocAppl {
      protected:
	cMessage * delayTimer;

	virtual void handleSelfMsg(cMessage *);

      public:
	Module_Class_Members(TestLocAppl, BaseLocAppl, 0);

	enum APPL_MSG_TYPES {
		SEND_BROADCAST_TIMER
	};

	virtual void initialize(int);
	virtual void finish();
};

#endif
