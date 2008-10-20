package org.mixim.examples.phyLayer;

import org.mixim.base.modules.BaseMobility;
import org.mixim.base.modules.BaseUtility;


module HybridHost
{
    parameters:
        double numHosts; // total number of hosts in the network

        @display("bgb=200,200,white,,;bgp=10,10");
    gates:
        input GSMRadioIn;
        input ISMRadioIn;

    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=70,50;i=block/blackboard");
        }
        mobility: BaseMobility {
            parameters:
                @display("p=150,50;i=block/cogwheel");
        }
        ISMNic: PhyMacNic {
            parameters:
                @display("b=42,42,rect,green,,;p=70,150;i=block/ifcard");
        }
        GSMNic: PhyMacNic {
            parameters:
                @display("b=42,42,rect,red,,;p=150,150;i=block/ifcard");
        }
    connections:

        GSMRadioIn --> GSMNic.radioIn;
        ISMRadioIn --> ISMNic.radioIn;

}

