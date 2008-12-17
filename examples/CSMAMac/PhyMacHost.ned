package org.mixim.examples.CSMAMac;

import org.mixim.base.modules.BaseMobility;
import org.mixim.base.modules.BaseUtility;


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
        mobility: BaseMobility {
            parameters:
                @display("p=130,130;i=block/cogwheel");
        }
        net: SimpleNetwLayer {
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

        radioIn --> nic.radioIn;

}

