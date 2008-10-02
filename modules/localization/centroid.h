/* -*- mode:c++ -*- ********************************************************
 * file:        Centroid.h
 *
 * author:      Aline Baggio
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
 * description: centroid class for the localization module
 **************************************************************************/

#ifndef CENTROID_H
#define CENTROID_H

#include "BaseLocalization.h"
#include "Location.h"
#include "LocPkt_m.h"
#include "Coord.h"
#include "Timer.h"

/* Should be in a network configuration file */
#define ANCHOR_TIMER_INTERVAL	2.0
#define NODE_TIMER_INTERVAL	4.0
//#define NBANCHORS 5
#define MIN_ANCHOR_POSITIONS 3 // 2D


typedef enum { SEND_ANCHOR_POS_TIMER = 0,
	       SEND_NODE_LOC_TIMER } CentroidTimer;

/**
 * @brief Centroid class for the localization module
 *
 * @author Aline Baggio
 */
class Centroid:public BaseLocalization, public Timer {
      public:
	//Module_Class_Members(Centroid, BaseLocalization, 0);

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();

//    enum LOC_MSG_TYPES{
//        SEND_ANCHOR_POS_TIMER,
//	SEND_NODE_LOC_TIMER,
//	ANCHOR_BROADCAST_MESSAGE,
//	NODE_POSITION_MESSAGE
//    };

protected:
    LocPkt *anchorTimer;
    LocPkt *nodeTimer;
    int nb_anchor_positions;

  //Location anchor_positions[NBANCHORS];

  enum { ANCHOR_BROADCAST_MESSAGE = APPLICATION_MSG + 1,
	 NODE_BROADCAST_MESSAGE};


protected:
    /** @brief Handle self messages such as timer... */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle messages from lower layer */
    //virtual void handleLowerMsg(cMessage*);
  virtual void handleMsg(cMessage*);
  
    /** @brief send a broadcast packet to all connected neighbors */
    void sendBroadcast(LocPkt *pkt);

    /** @brief send a reply to a broadcast message */
    //void sendReply(ApplPkt *msg);  

    virtual void handleTimer(unsigned int count);
};

#endif				/* CENTROID_H */
