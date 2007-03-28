/*
 *	copyright:   	(C) 2006 Computer Networks Group (CN) at
 *			University of Paderborn, Germany.
 *	
 *			This program is free software; you can redistribute it
 *			and/or modify it under the terms of the GNU General Public
 *			License as published by the Free Software Foundation; either
 *			version 2 of the License, or (at your option) any later
 *			version.
 *
 *			For further information see file COPYING
 *			in the top level directory.
 *
 *			Based on Mobility Framework 2.0p2 developed at 
 *			TKN (TU Berlin) and, ChSim 2.1 developed at CN 
 *			(Uni Paderborn).
 *
 *	file:		$RCSfile: StreamApplLayer.h,v $
 *
 *      last modified:	$Date: 2007/02/21 15:41:53 $
 *      by:		$Author: tf $
 *
 *      informatin:	-
 *
 *	changelog:   	$Revision: 1.6 $
 *			$Log: StreamApplLayer.h,v $
 *			Revision 1.6  2007/02/21 15:41:53  tf
 *			- reply is not sent as broadcast any longer
 *			
 *			Revision 1.5  2007/01/31 09:45:05  tf
 *			- added parameter streamRate to set rate of stream generation
 *			
 *			Revision 1.4  2007/01/21 20:36:15  tf
 *			- packet generation rate is altered by MAC buffer state, to prevent
 *			  empty MAC buffer
 *			- only ApplLayer with initBurst>0 acts as stream generator
 *			
 *			Revision 1.3  2007/01/19 15:43:54  tf
 *			- fixed timer initialization in StreamApplLayer
 *			- fixed MAC frame duration computation
 *			
 *			Revision 1.2  2007/01/19 14:50:44  tf
 *			- fixed stupid typo in function name
 *			
 *			Revision 1.1  2007/01/19 09:57:07  tf
 *			- added new Applayer for tests
 *				- params
 *					length of frames
 *				- sends a broadcast which is answered by all receiving stations
 *				  receiving stations reply and will be scheduled as stream dest.
 *			
 */


#ifndef STREAMAPPL_LAYER_H
#define STREAMAPPL_LAYER_H

#include "TestApplLayer.h"
#include "NetwControlInfo.h"
#include <Mac80211a.h>

#include <list>
#include <map>

/**
 * @brief Application layer to test lower layer implementations
 *
 * The test layer send an initial burst and saves source addresses of the replies
 * which are used to generate a stream to each station with given length
 **/

typedef std::list<int> ClientList;
typedef std::map<int,int> ClientAddresses;

class StreamApplLayer : public TestApplLayer
{
 public:
  /** @brief Called by the Blackboard whenever a change occurs we're interested in */
  virtual void receiveBBItem(int category, BBItem *details, int scopeModuleId);
  Module_Class_Members( StreamApplLayer, TestApplLayer, 0 );
  virtual void initialize(int);
  virtual void finish();

 protected:
  virtual void handleSelfMsg(cMessage*);
  virtual void handleLowerMsg(cMessage*);
  virtual void sendTo(int);
  virtual void sendReply(ApplPkt* msg, NetwControlInfo* nci);
  int streamLength;
  double streamRate;
  int packetSize;
  int headerLength;
  int initBurst;
  ClientList clientList;
  ClientAddresses clientNetwAddresses;
  cMessage* startTimer;

  /** @brief buffer state information from MAC */
  BBBufferState* bufferState;
  int bufferStateCat;
  double randDelay;
};

#endif
 
