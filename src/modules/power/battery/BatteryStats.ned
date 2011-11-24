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

package org.mixim.modules.power.battery;

//
// module for collecting battery statistics
//
simple BatteryStats
{
    parameters:
        bool notAffectedByHostState = default(true);
        bool debug = default(false);
        // write per-activity and per-device statistics to omnetpp.sca
		// (if false, only total energy and lifetime are written)
        bool detail = default(true);
        // subscribe to battery status and record time series
		// data in omnetpp.vec (resolution depends on battery's 
		// publishDelta)
        bool timeSeries = default(false);
}
