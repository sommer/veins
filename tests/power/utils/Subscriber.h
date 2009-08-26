/* -*- mode:c++ -*- ********************************************************
 * Energy Framework for Omnet++, version 0.9
 *
 * Author:  Laura Marie Feeney
 *
 * Copyright 2009 Swedish Institute of Computer Science.
 *
 * This software is provided `as is' and without any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.
 *
 ***************************************************************************/
#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <omnetpp.h>
#include "BaseModule.h"

#include "HostState.h"

/**
 * @brief trivial test module that subscribes to HostState and logs
 * host failure notification
 */
class Subscriber : public BaseModule
{
public:
  virtual void initialize(int);
  virtual void handleMessage( cMessage* );
  virtual void handleHostState(const HostState& state);
  virtual void finish( );

 protected:

};
#endif
