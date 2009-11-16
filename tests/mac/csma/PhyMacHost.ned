package org.mixim.tests.mac.csma;

import org.mixim.modules.mobility.ConstSpeedMobility;
//import org.mixim.modules.application.BurstApplLayer;
import org.mixim.base.modules.BaseUtility;
import org.mixim.base.modules.BaseArp;
import org.mixim.tests.mac.utils.NetworkStackTrafficGen;


module PhyMacHost
{
    parameters:
        double numHosts; // total number of hosts in the network

        @display("bgb=363,335,white;bgp=10,10");
    gates:
        input radioIn;

    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=205,182;i=block/blackboard");
        }
        mobility: ConstSpeedMobility {
            parameters:
                @display("p=205,81;i=block/cogwheel");
        }
        arp: BaseArp {
            @display("p=205,240;b=30,25");
        }
        appl: NetworkStackTrafficGen {
            parameters:
                @display("p=80,132;b=80,30,rect,white");
        }
        nic: PhyMacNic {
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

