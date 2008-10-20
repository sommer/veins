package org.mixim.tests.basePhyLayer;

import org.mixim.base.modules.BaseMobility;
import org.mixim.base.modules.BaseUtility;


module TestHost
{
    parameters:
        double numHosts; // total number of hosts in the network

        @display("bgb=180,200,white,,;bgp=10,10");
    gates:
        input radioIn;

    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=130,60;b=25,25,,black,,");
        }
        mobility: BaseMobility {
            parameters:
                @display("p=130,130;i=cogwheel2");
        }
        nic: TestNic {
            parameters:
                @display("b=32,30;p=60,166;i=iface");
        }
    connections:

        radioIn --> nic.radioIn;

}

