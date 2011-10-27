package org.mixim.modules.node;

import org.mixim.base.modules.IBaseMobility;
import org.mixim.base.modules.BaseArp;
import org.mixim.base.modules.IBaseApplLayer;
import org.mixim.base.modules.IBaseNetwLayer;
import org.mixim.modules.nic.Nic80211;

//
// Example host module using an 802.11 Network interface card.
//
module Host80211
{
    parameters:
        string applType; //type of the application layer
        string netwType; //type of the network layer
        string mobType; //type of the mobility module
        @display("bgb=40,,white;i=device/laptop;b=40,40,rect");
    gates:
        input radioIn; // gate for sendDirect
    submodules:
        arp: BaseArp {
            parameters:
                @display("p=130,108;b=24,24,rect,blue;i=block/process");
        }
        mobility: <mobType> like IBaseMobility {
            parameters:
                @display("p=130,172;i=block/cogwheel");
        }
        appl: <applType> like IBaseApplLayer {
            parameters:
                @display("p=60,50;i=app");
        }
        net: <netwType> like IBaseNetwLayer {
            parameters:
                @display("p=60,108;i=block/layer");
        }
        nic: Nic80211 {
            parameters:
                @display("p=60,166;i=block/ifcard");
        }
    connections:
        nic.upperLayerOut --> net.lowerLayerIn;
        nic.upperLayerIn <-- net.lowerLayerOut;
        nic.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> net.lowerControlIn;
        nic.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- net.lowerControlOut;

        net.upperLayerOut --> appl.lowerLayerIn;
        net.upperLayerIn <-- appl.lowerLayerOut;
        net.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.lowerControlIn;
        net.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;

}

