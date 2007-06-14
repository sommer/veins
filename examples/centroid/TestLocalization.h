/* -*- mode:c++ -*- ********************************************************
 * file:        TestLocalization.h
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
 * description: test class for the localization module
 **************************************************************************/

#ifndef TEST_LOCALIZATION_H
#define TEST_LOCALIZATION_H

#include "BaseLocalization.h"
#include "localization.h"
#include "PositionPkt_m.h"
#include "Coord.h"

/* Should be in a network configuration file */
#define ANCHOR_TIMER_INTERVAL	2.0
#define NODE_TIMER_INTERVAL	4.0
#define NBANCHORS 7
#define MIN_ANCHOR_POSITIONS 3 // 2D

/**
 * @brief Test class for the localization module
 *
 * @author Peterpaul Klein Haneveld
 */
class TestLocalization:public BaseLocalization {
      public:
	Module_Class_Members(TestLocalization, BaseLocalization, 0);

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();

    enum LOC_MSG_TYPES{
        SEND_ANCHOR_POS_TIMER,
	SEND_NODE_LOC_TIMER,
	ANCHOR_BROADCAST_MESSAGE,
	NODE_POSITION_MESSAGE
    };

protected:
    LocPkt *anchorTimer;
    LocPkt *nodeTimer;
    int nb_anchor_positions;
    PositionData anchor_positions[NBANCHORS];

protected:
    /** @brief Handle self messages such as timer... */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage*);
  
    /** @brief send a broadcast packet to all connected neighbors */
    void sendBroadcast(PositionPkt *pkt);

    /** @brief send a reply to a broadcast message */
    //void sendReply(ApplPkt *msg);  
};

#endif				/* TEST_LOCALIZATION_H */
