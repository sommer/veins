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
/*
 * per-device, per-activity energy data is shared between Battery and
 * BatteryStats using the DeviceEntry data structure
 *
 ***************************************************************************/

#ifndef DEVICEENTRY_H
#define DEVICEENTRY_H

/** @brief per-device/per-account record of battery consumption, is
 * passed to BatteryStats on finish()
 *
 * For each device, maintain the current value of the current
 * being drawn by the device, the activity and time for which ongoing
 * current draw is being charged. The sum over the activities is the
 * total energy consumed by the device, but the sum of times is the
 * total active time, not ncessarily the total lifetime.
 *
 * @ingroup power
 */
class DeviceEntry
  {
  public:
    opp_string  name;
    double 	draw;
    int		currentActivity;
    int 	numAccts;
    double	*accts;
    simtime_t	*times;

    DeviceEntry() {
      name = NULL;
      draw = 0.0;
      numAccts = 0;
      currentActivity = -1;
      accts = NULL;
      times = NULL;
    }

    ~DeviceEntry() {
      delete []  accts;
      delete [] times;
    }
  };

#endif

//
