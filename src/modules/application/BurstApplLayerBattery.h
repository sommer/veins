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


#ifndef BURST_APPL_LAYER_BATTERY_H
#define BURST_APPL_LAYER_BATTERY_H

#include "MiXiMDefs.h"
#include "BurstApplLayer.h"
#include "HostState.h"

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
class MIXIM_API BurstApplLayerBattery : public BurstApplLayer
{
 public:

  /** @brief Initialite module parameters*/
  virtual void initialize(int);

  virtual void finish() ;

 protected:
  virtual void handleHostState(const HostState& state);
  virtual void handleLowerMsg( cMessage* );
  virtual void handleSelfMsg(cMessage *);

 private:

  /** @brief The number of broadcasts queued so far.*/
  int bcastOut;
  /** @brief The number of replies sent so far.*/
  int replyOut;
  /** @brief The number of replies received so far.*/
  int replyIn;

};

#endif

