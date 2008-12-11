package org.mixim.examples.foxtrot;

import org.mixim.base.modules.BaseMacLayer;
import org.mixim.base.phyLayer.BasePhyLayer;


module Nic
{
    parameters:
        string phyType;
        @display("bgb=101,180,white,,;bgp=10,10");
    gates:
        input upperGateIn; // to upper layers
        output upperGateOut; // from upper layers
        input upperControlIn; // control to upper
        output upperControlOut; // control from upper
        input radioIn; // radioIn gate for sendDirect
    submodules:
        mac: BaseMacLayer {
            parameters:
                @display("p=60,50;i=prot2");
        }
        phy: BasePhyLayer {
            parameters:
                @display("p=60,150;i=prot3");
        }
    connections:
        phy.upperGateOut --> mac.lowerGateIn;
        phy.upperGateIn <-- mac.lowerGateOut;
        phy.upperControlOut --> { @display("ls=red;m=m,50,50,75,0"); } --> mac.lowerControlIn;
        phy.upperControlIn <-- { @display("ls=red;m=m,25,0,25,0"); } <-- mac.lowerControlOut;

        mac.upperGateOut --> upperGateOut;
        mac.upperGateIn <-- upperGateIn;
        mac.upperControlOut --> { @display("ls=red;m=m,80,0,60,0"); } --> upperControlOut;
        mac.upperControlIn <-- { @display("ls=red;m=m,60,0,80,0"); } <-- upperControlIn;

        radioIn --> phy.radioIn;

}

