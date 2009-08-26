//# Energy Framework for Omnet++, version 0.9
//#
//# Author:  Laura Marie Feeney 
//# 
//# Copyright 2009 Swedish Institute of Computer Science.  
//# 
//# This software is provided `as is' and without any express or implied
//# warranties, including, but not limited to, the implied warranties of
//# merchantability and fitness for a particular purpose.
//#
//######################################################################
// module for collecting battery statistics
// 
//      detail: write per-activity and per-device statistics to omnetpp.sca
//              (if false, only total energy and lifetime are written)
//
//      timeSeries: subscribe to battery status and record time series
//              data in omnetpp.vec (resolution depends on battery's 
//              publishDelta)
package org.mixim.modules.power.battery;

simple BatteryStats
{
    parameters:
        bool notAffectedByHostState = true;
        bool debug;
        bool detail;
        bool timeSeries;
}
