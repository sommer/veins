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
#ifndef DEVICEDUTYSIMPLE_H
#define DEVICEDUTYSIMPLE_H

#include <omnetpp.h>
#include "BatteryAccess.h"

#include "BatteryState.h"
#include "HostState.h"

/**
 * @brief A trivial device with simple on/off duty cycle

 * A trivial device module that has an duty on/off cycle and a fixed
 * wakeup cost.  Subscribes to Host State and ends duty cylcle on host
 * failure.  Used for testing, can also be used as e.g. a prototype
 * sensor device. See DeviceDutySimple.ned for parameters.
 */
class DeviceDutySimple : public BatteryAccess
{

public:
  virtual void initialize(int);
  virtual void handleMessage( cMessage* );
  virtual void handleHostState(const HostState& state);
  virtual void finish();

protected:

  enum DeviceState {
    ON,
    OFF
  };

  cMessage *on, *off;

  double period, dutyCycle, current, wakeup;
};

#endif // DEVICEDUTYSIMPLE_H
