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
// This host NED module can be used to study ultra-wideband impulse radio
// wireless communication systems using IEEE 802.15.4A compatible transceivers.
// 
// @author Jérôme Rousselot
// 
// See the following publications for more information:
// [1] An Ultra-Wideband Impulse Radio PHY Layer Model for Network Simulation,
// J. Rousselot, J.-D. Decotignie. Simulation: Transactions of the Society
// for Computer Simulation, vol. 87 no. 1-2 pp.82-112, January 2011.
// <a href="http://dx.doi.org/10.1177/0037549710377767">http://dx.doi.org/10.1177/0037549710377767</a>
// [2] A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
// for Network Simulation, J. Rousselot, J.-D. Decotignie,
// Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
// <a href="http://portal.acm.org/citation.cfm?id=1537714">http://portal.acm.org/citation.cfm?id=1537714</a>
// [3] Ultra Low Power Communication Protocols for UWB Impulse Radio Wireless Sensor Networks,
// J. Rousselot. EPFL thesis 4720, 2010.
// <a href="http://infoscience.epfl.ch/record/147987">http://infoscience.epfl.ch/record/147987</a> 
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

