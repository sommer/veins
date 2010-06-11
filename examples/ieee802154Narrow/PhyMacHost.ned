package org.mixim.examples.ieee802154Narrow;

import org.mixim.modules.mobility.LinearMobility;
//import org.mixim.modules.application.BurstApplLayer;
import org.mixim.base.modules.BaseUtility;
import org.mixim.base.modules.BaseArp;
import org.mixim.base.modules.BaseMobility;
import org.mixim.modules.power.battery.SimpleBattery;
import org.mixim.modules.power.battery.BatteryStats;

import org.mixim.modules.nic.Nic802154_TI_CC2420;

module PhyMacHost
{
    parameters:
        double numHosts; // total number of hosts in the network

        @display("bgb=363,335,white;bgp=10,10");
    gates:
        input radioIn;

    submodules:
        batteryStats: BatteryStats {
            @display("p=239,206;i=block/circle");
        }
        battery: SimpleBattery {
            @display("p=171,206;i=block/control");
        }
        utility: BaseUtility {
            parameters:
                @display("p=205,146;i=block/blackboard");
        }
        mobility: BaseMobility {
            parameters:
                @display("p=205,81;i=block/cogwheel");
        }
        arp: BaseArp {
            @display("p=205,264;b=30,25;i=block/process");
        }
        appl: NetworkStackTrafficGen {
            parameters:
                @display("p=80,132;b=20,20,rect,white;i=block/app");
        }
        nic: Nic802154_TI_CC2420 {
            parameters:
                @display("b=32,30;p=80,233;i=block/ifcard");
        }
    connections:
        nic.upperGateOut --> { @display("ls=black;m=m,25,50,25,0"); } --> appl.lowerGateIn;
        nic.upperGateIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- appl.lowerGateOut;
        nic.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> appl.lowerControlIn;
        nic.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- appl.lowerControlOut;
        radioIn --> nic.radioIn;

}

