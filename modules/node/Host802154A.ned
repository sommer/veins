package org.mixim.modules.node;

import org.mixim.base.modules.BaseMobility;
import org.mixim.base.modules.BaseUtility;
import org.mixim.modules.application.TestApplication;
import org.mixim.modules.netw.DummyRoute;
import org.mixim.modules.netw.ArpHost;
import org.mixim.modules.utility.phyPER;
import org.mixim.modules.nic.Nic802154A;
import org.mixim.modules.power.battery.SimpleBattery;
import org.mixim.modules.power.battery.BatteryStats;

module Host802154A
{
    parameters:
        double numHosts; // total number of hosts in the network

        @display("bgb=356,318,white;bgp=10,10");
    gates:
        input radioIn;

    submodules:
         utility: BaseUtility {
            parameters:
                @display("p=292,60;i=block/blackboard");
        }
        stats: phyPER {
            @display("p=202,257");
        }
        batteryStats: BatteryStats {
            @display("p=292,257;i=block/circle");
        }
        battery: SimpleBattery {
            @display("p=292,186;i=block/control");
        }
        mobility: BaseMobility {
            parameters:
                @display("p=292,120;i=block/cogwheel");
        }
        nic: Nic802154A {
            parameters:
                @display("b=32,30;p=86,265;i=block/ifcard");
        }
        net: DummyRoute {
            parameters:
                @display("p=86,159");
        }
        arp: ArpHost {
            parameters:
                @display("p=202,186");
        }
        app: TestApplication {
            parameters:
                @display("p=86,60;i=block/app");

        }

    connections:
        net.lowerGateOut --> nic.upperGateIn;
        net.lowerGateIn <-- nic.upperGateOut;
        net.lowerControlOut --> nic.upperGateCtrlIn;
        net.lowerControlIn <-- nic.upperGateCtrlOut;

        net.upperGateOut --> app.lowerGateIn;
        net.upperGateIn <-- app.lowerGateOut;
        net.upperControlOut --> app.lowerControlIn;
        net.upperControlIn <-- app.lowerControlOut;


        radioIn --> nic.radioIn;



}

