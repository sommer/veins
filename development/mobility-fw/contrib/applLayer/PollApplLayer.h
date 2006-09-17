/* -*- mode:c++ -*- ********************************************************
 * file:        PollApplLayer.h
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: application layer for a simple bse station who polls all 
 *              clients in a round-robin manner
 **************************************************************************/


#ifndef POLL_APPL_LAYER_H
#define POLL_APPL_LAYER_H

#include "ClientApplLayer.h"
#include <map>


/**
 * @brief Base station application layer to test centralized scenarios
 *
 * Application layer for a base station in a centralized WLAN-like
 * network. Every '@ref broadcastInterval' it broadcasts a ping
 * message to determine the reachable clients. Afterwards it polls all
 * reachable clients in a round-robin manner, where '@ref pollTimeout'
 * is the reply timeout for the clients.
 *
 * @sa ClientApplLayer
 * @ingroup applLayer
 * @author Daniel Willkomm
 **/
class PollApplLayer : public ClientApplLayer
{
 public:
  Module_Class_Members( PollApplLayer, ClientApplLayer, 0 );
  /** @brief Initialite module parameters*/
  virtual void initialize(int);

  virtual void finish();

 protected:
  /** @brief Number of clients in the network*/
  int numClients;

  /** @brief Number of alive clients in the network*/
  int aliveClients;

  /** @brief Time between broadcasts in seconds*/
  double broadcastInterval;

  /** @brief Timeout for poll messages in seconds*/
  double pollTimeout;

  typedef std::map<int,bool> cAliveMap;
  typedef std::map<int,int> cNetwAddrMap;
  typedef cAliveMap::iterator cAliveMapIterator;

  /** @brief Map to store all reachable clients*/
  cAliveMap clients;

    /** @brief Map to store the netw addresses o fthe clients */
    cNetwAddrMap clientAddrs;

  /** @brief Iterator for the clients map*/
  cAliveMapIterator it;

  /** @brief Timer message for polling*/
  cMessage* pollTimer;


 /** @brief Handle messages from lower layer... */
  virtual void handleLowerMsg(cMessage*);

  /** @brief Handle self messages such as timer... */
  virtual void handleSelfMsg(cMessage*);
  
  /** @brief Send a broadcast packet to all connected neighbors */
  void sendBroadcast();

  /** @brief Send a poll message to 'addr'*/
  void sendPoll(int);

  /** @brief Poll next client in the list*/
  void pollNext();
};

#endif
 
