package org.mixim.examples.IEEE802154;

import org.mixim.modules.mobility.ConstSpeedMobility;
import org.mixim.modules.application.BurstApplLayer;
import org.mixim.base.modules.BaseUtility;
import org.mixim.base.modules.BaseArp;
import org.mixim.base.modules.BaseNetwLayer;


module PhyMacHost
{
    parameters:
        double numHosts; // total number of hosts in the network

        @display("bgb=180,200,white,,;bgp=10,10");
    gates:
        input radioIn;

    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=130,60;i=block/blackboard");
        }
        mobility: ConstSpeedMobility {
            parameters:
                @display("p=130,130;i=block/cogwheel");
        }
        appl: BurstApplLayer {
            @display("p=60,0;b=80,30,rect,white,,");
        }
        arp: BaseArp {
            @display("p=200,225;b=30,25");
        }
        net: BaseNetwLayer {
            parameters:
                @display("p=60,10;b=80,30,rect,white,,");
        }
        nic: PhyMacNic {
            parameters:
                @display("b=32,30;p=60,166;i=block/ifcard");
        }
    connections:
        nic.upperGateOut --> { @display("ls=black;m=m,25,50,25,0"); } --> net.lowerGateIn;
        nic.upperGateIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- net.lowerGateOut;
        nic.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> net.lowerControlIn;
        nic.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- net.lowerControlOut;
        net.upperGateOut --> { @display("ls=black;m=m,25,50,25,0"); } --> appl.lowerGateIn;
        net.upperGateIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- appl.lowerGateOut;
        net.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> appl.lowerControlIn;
        net.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;

}

