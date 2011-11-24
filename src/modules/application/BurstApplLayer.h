/* -*- mode:c++ -*- ********************************************************
 * file:        BurstApplLayer.h
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
 * description: application layer like the TestApplLayer but sends burst of
 *               messages
 **************************************************************************/


#ifndef BURST_APPL_LAYER_H
#define BURST_APPL_LAYER_H

#include "MiXiMDefs.h"
#include <TestApplLayer.h>


/**
 * @brief Application layer to test lower layer implementations
 *
 * This application layer does exactly the same as the
 * TestApplLayer. The only difference is that is sends a burst of
 * broadcast messages instead of just one. The burst size is specified
 * in burstSize and can be set in omnetpp.ini
 *
 * @sa TestApplALyer
 * @ingroup applLayer
 * @author Daniel Willkomm
 **/
class MIXIM_API BurstApplLayer : public TestApplLayer
{
 public:
  /** @brief Initialize module parameters*/
  virtual void initialize(int);

 protected:
  /** @brief Handle self messages such as timer... */
  virtual void handleSelfMsg(cMessage*);

  /** @brief Handle messages from lower layer */
  virtual void handleLowerMsg(cMessage*);

  /** @brief Number of messages to send in a burst*/
  int  burstSize;
  /** @brief If true, send a unicast BROADCAST_REPLY message to each
   * received BROADCAST message. */
  bool bSendReply;
};

#endif

