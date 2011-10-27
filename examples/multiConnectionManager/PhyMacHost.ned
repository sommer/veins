package org.mixim.examples.multiConnectionManager;

import org.mixim.base.modules.BaseMobility;
import org.mixim.base.modules.BaseArp;
import org.mixim.base.modules.BaseNetwLayer;
import org.mixim.modules.application.BurstApplLayer;


module PhyMacHost
{
    parameters:
        double numHosts; // total number of hosts in the network
        @display("bgb=180,200,white,,;bgp=10,10");
    gates:
        input radioIn;

    submodules:
        mobility: BaseMobility {
            parameters:
                @display("p=130,130;i=block/cogwheel");
        }
        arp: BaseArp {
            @display("p=200,225;b=30,25");
        }
        appl: BurstApplLayer {
            @display("p=60,42;b=10,10,rect;i=block/app");
        }
        net: BaseNetwLayer {
            parameters:
                @display("p=60,106;b=10,10,rect,white;i=block/layer");
        }
        nic: PhyMacNic {
            parameters:
                @display("b=32,30;p=60,166;i=block/ifcard");
        }
    connections:
        nic.upperLayerOut --> { @display("ls=black;m=m,25,50,25,0"); } --> net.lowerLayerIn;
        nic.upperLayerIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- net.lowerLayerOut;
        nic.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> net.lowerControlIn;
        nic.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- net.lowerControlOut;

        net.upperLayerOut --> appl.lowerLayerIn;
        net.upperLayerIn <-- appl.lowerLayerOut;
        net.upperControlOut --> appl.lowerControlIn;
        net.upperControlIn <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;

}

