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
//#######################################################################
// a simple battery module
//
//      numDevices: number of modules that will draw energy from the 
//              battery (i.e. number of input gates set in Host.ned) 
//
//      capacity (mA-H): battery capacity
//      nominal(mA-H): nominal battery capacity
//      voltage (V): nominal voltage 
//      resolution (time): capacity is updated at least every resolution time
//      publishDelta(0..1): capacity is published to the BB each time it is
//              observed to have changed by publishDelta * nominal_capacity
//      publishTime: if > 0, capacity is published to the BB each publishTime 
//              interval,

// Note that resolution should be much smaller than the expected time to
// first node failure.

package org.mixim.modules.power.battery;

import org.mixim.base.modules.BaseBattery;

simple SimpleBattery extends BaseBattery
{    
    parameters:
        @class(SimpleBattery);
        bool debug;
        volatile double nominal;
        double capacity;
        volatile double voltage;
        volatile double resolution @unit(s);
        volatile double publishDelta;
        volatile double publishTime @unit(s);
        double numDevices;

}
