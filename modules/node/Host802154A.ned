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

//
// See the following publications for more information:
// [1] An Ultra Wideband Impulse Radio PHY Layer Model for Network Simulation,
// J. Rousselot, J.-D. Decotignie, Simulation: Transactions of the Society
// for Computer Simulation, 2010 (submitted).
// [2] A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
// for Network Simulation, Jérôme Rousselot, Jean-Dominique Decotignie,
// Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
// http://portal.acm.org/citation.cfm?id=1537714
//
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
        net.lowerControlOut --> nic.upperControlIn;
        net.lowerControlIn <-- nic.upperControlOut;

        net.upperGateOut --> app.lowerGateIn;
        net.upperGateIn <-- app.lowerGateOut;
        net.upperControlOut --> app.lowerControlIn;
        net.upperControlIn <-- app.lowerControlOut;


        radioIn --> nic.radioIn;



}

