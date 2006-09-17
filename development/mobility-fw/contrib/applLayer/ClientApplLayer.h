/* -*- mode:c++ -*- ********************************************************
 * file:        ClientApplLayer.h
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
 * description: application layer: test client for the application layer
 *              in a centrally controlled network
 **************************************************************************/


#ifndef CLIENT_APPL_LAYER_H
#define CLIENT_APPL_LAYER_H

#include "BasicApplLayer.h"


/**
 * @brief Client application layer to test centrallized scenarios.
 * 
 * This application layer just replies to broadcast and poll messages
 * from a base station (@ref PollApplLayer)
 *
 * @ingroup applLayer
 * @sa PollApplLayer
 * @author Daniel Willkomm
 **/
class ClientApplLayer : public BasicApplLayer
{
 public:
  Module_Class_Members( ClientApplLayer, BasicApplLayer, 0 );

  /** @brief Possible application message types for this
      implementation.*/
  enum APPL_MSG_TYPES{
    SEND_BROADCAST_TIMER,
    POLL_REPLY_TIMEOUT,
    BROADCAST_REPLY_TIMEOUT,
    BROADCAST_MESSAGE,
    BROADCAST_REPLY_MESSAGE,
    POLL_MESSAGE,
    POLL_REPLY
  };

 protected:
  /** @brief Handle messages from lower layer */
  virtual void handleLowerMsg(cMessage*);  
};

#endif
