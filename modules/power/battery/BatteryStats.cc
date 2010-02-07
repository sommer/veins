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
 * Battery Stats collects and formats times series and summary data
 * from the Battery.
 *
 * BatteryStats' summary() and detail() methods are called by
 * Battery's finish() to format its table of device battery
 * consumption. (Methods shoud manage their own resources, since own
 * finish() may be done).
 *
 * For time series data it subscribes to BatteryState published by the
 * Battery.  Note: only BatteryStats should do this, all other modules
 * should use the Battery's estimateResidual() method (Battery Stats
 * does this as well, but for now estimate just follows redisual
 * capacity.)
 */
#include "BatteryStats.h"
#include <iostream>

Define_Module(BatteryStats);

void BatteryStats::initialize(int stage)
{
  BaseModule::initialize(stage);

  if (stage==0) {
    doDetail = 0;
    doDetail = par("detail").boolValue();
    EV << "show details = " << doDetail << endl;

    doTimeSeries = 0;
    doTimeSeries = par("timeSeries").boolValue();
    EV << "show timeSeries = " << doTimeSeries << endl;

    batteryCat = -1;
    if (doTimeSeries) {
      int scopeHost = (this->findHost())->getId();
      BatteryState bs;
      batteryCat = utility->subscribe(this, &bs, scopeHost);

      // suggest enabling only residualVec (omnetpp.ini), unless
      // others are of interest

      residualVec.setName("capacity");
      residualVec.setUnit("mW-s");
      relativeVec.setName("capacity(relative)");

      BaseBattery* batteryModule = FindModule<BaseBattery*>::findSubModule(getParentModule());
      if (batteryModule) {
        battery = batteryModule;
      }
      else {
        error("no battery module found, please check your Host.ned");
      }
      estimateVec.setName("estimate");
      estimateVec.setUnit("mW-s");
      estimateRelVec.setName("estimate(relative)");
    }
  }
}

void BatteryStats::handleMessage(cMessage *msg)
{
  error("BatteryStats doesn't handle any messages");
}

// summary() and detail() are invoked by Battery's finish() method

void BatteryStats::summary(double init, double final, simtime_t lifetime)
{
  Enter_Method_Silent();
  recordScalar("nominal", init, "mW-s");
  recordScalar("total", init - final, "mW-s");
  recordScalar("lifetime", lifetime, "s");
  recordScalar("Mean power consumption", (init - final)/simTime().dbl(), "mW");
}

void BatteryStats::detail(DeviceEntry *devices, int numDevices)
{
  Enter_Method_Silent();
  if (!doDetail)
    return;

  recordScalar("num devices", numDevices);

  for (int i = 0; i < numDevices; i++){
    double total = 0;
    for (int j = 0; j < devices[i].numAccts; j++) {
      total += devices[i].accts[j];
    }
    recordScalar(devices[i].name.c_str(), i);
    recordScalar("device total (mWs)", total);
    for (int j = 0; j < devices[i].numAccts; j++) {
      recordScalar("account", j);
      recordScalar("energy (mWs)", devices[i].accts[j]);
      recordScalar("time (s)", devices[i].times[j]);
    }
  }
}

void BatteryStats::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    Enter_Method_Silent();
    BaseModule::receiveBBItem(category, details, scopeModuleId);

    if (category == batteryCat) {
      double residualCapacity;
      double relativeCapacity;

      // battery time series never publishes capacity < 0, just 0
      residualCapacity = ((BatteryState *)details)->getAbs();
      residualVec.record(residualCapacity);
      relativeCapacity = ((BatteryState *)details)->getRel();
      relativeVec.record(relativeCapacity);

      // for comparison, also get the estimated residual capacity
      double estimate = battery->estimateResidualAbs();
      estimateVec.record(estimate);

      double estimateRel = battery->estimateResidualRelative();
      estimateRelVec.record(estimateRel);
    }

}

void BatteryStats::finish() {
}


