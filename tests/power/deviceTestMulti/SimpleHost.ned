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
// empty host to test energy framework.  see README.txt for details.
package org.mixim.tests.power.deviceTestMulti;

import org.mixim.tests.power.utils.DeviceDutySimple;
import org.mixim.modules.power.battery.SimpleBattery;
import org.mixim.modules.power.battery.BatteryStats;

module SimpleHost
{
    parameters:
        @display("bgb=250,250,$color,,;bgp=10,10");
    submodules:
        firstDevice: DeviceDutySimple {
            parameters:
                @display("p=200,160;b=30,25");
        }
        secondDevice: DeviceDutySimple {
            parameters:
                @display("p=125,100;b=30,25");
        }
        battery: SimpleBattery {
            parameters:
                numDevices = 2;
                @display("p=125,100;b=30,25");
        }
        batteryStats: BatteryStats;

}
