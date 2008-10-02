/* -*- mode:c++ -*- ********************************************************
 * file:        DetailedPhy.h
 * 
 * author:      Tom Parker
 *
 * copyright:   (C) 2007 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; 
 *              version 2 of the License.
 *
 *              For further information see file COPYING 
 *              in the top level directory
 *
 * description: phy layer - requires and sends detailed control messages
 ***************************************************************************/

#ifndef DETAILED_PHY_H
#define DETAILED_PHY_H

#include "CollisionsPhy.h"


class DetailedPhy: public CollisionsPhy
{
public:
    //Module_Class_Members(DetailedPhy, CollisionsPhy, 0 );

	void initialize(int stage);

protected:
	typedef enum {SLEEP=1, LISTEN, TRANSMIT} RadioState;
	RadioState state;

    virtual void handleLowerMsgStart(cMessage*);

    virtual void handleLowerMsgEnd(cMessage*);

    virtual void handleUpperMsg(cMessage*);

    virtual void handleUpperControl(cMessage*);

    virtual void handleCollision(cMessage *);
            
    //virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId) {Enter_Method_Silent();}
	virtual void increment();
	virtual void decrement();
};

#endif

