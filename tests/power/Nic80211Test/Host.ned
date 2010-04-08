//#
//# Energy Framework for Omnet++, version 0.9
//#
//# Author:  Laura Marie Feeney 
//# 
//# Copyright 2009 Swedish Institute of Computer Science.  
//# 
//# This software is provided `as is' and without any express or implied
//# warranties, including, but not limited to, the implied warranties of
//# merchantability and fitness for a particular purpose.
//######################################################################
// Tests the Nic80211Battery, the Energy Framework's battery aware
// 802.11 module
package org.mixim.tests.power.Nic80211Test;

import org.mixim.modules.application.BurstApplLayerBattery;
import org.mixim.base.modules.BaseNetwLayer;
import org.mixim.modules.nic.Nic80211Battery;
import org.mixim.base.modules.BaseArp;
import org.mixim.modules.mobility.ConstSpeedMobility;
import org.mixim.modules.power.battery.SimpleBattery;
import org.mixim.modules.power.battery.BatteryStats;
import org.mixim.base.modules.BaseUtility;

module Host

{
    parameters:
        @display("bgb=250,250,white,,;bgp=10,10");
    gates:
        input radioIn; // gate for sendDirect

    submodules:
        blackboard: BaseUtility {
            parameters:
                @display("p=200,75;b=30,25");

        }
        mobility: ConstSpeedMobility {
            parameters:
                @display("p=200,150;b=30,25");

        }
        battery: SimpleBattery {
            parameters:
                numDevices = 1;
                @display("p=60,200;b=30,25;rect");
        }
        batteryStats: BatteryStats {
            parameters:
                @display("p=120,200;b=30,25;rect");

        }
        appl: BurstApplLayerBattery {
            parameters:
                @display("p=90,50;b=100,20,rect");

        }
        arp: BaseArp {
            parameters:
                @display("p=200,225;b=30,25,rect");

        }
        net: BaseNetwLayer {
            parameters:
                @display("p=90,100;b=100,20,rect");

        }
        nic: Nic80211Battery {
            parameters:
                @display("p=90,150;b=100,20,rect");

        }
    connections:
        nic.upperGateOut --> net.lowerGateIn;
        nic.upperGateIn <-- net.lowerGateOut;
        nic.upperControlOut --> net.lowerControlIn;
        nic.upperControlIn <-- net.lowerControlOut;

        net.upperGateOut --> appl.lowerGateIn;
        net.upperGateIn <-- appl.lowerGateOut;
        net.upperControlOut --> appl.lowerControlIn;
        net.upperControlIn <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;

}
