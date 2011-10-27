package org.mixim.modules.node;

import org.mixim.modules.nic.IWirelessNic;
import org.mixim.base.modules.IBaseMobility;
import org.mixim.base.modules.BaseArp;
import org.mixim.base.modules.IBaseApplLayer;
import org.mixim.base.modules.IBaseNetwLayer;

//
// Example host module for a basic MiXiM host with configurable NIC, network 
// layer, application layer and mobility module. 
//
module HostBasic
{
    parameters:
        string applType; //type of the application layer
        string netwType; //type of the network layer
        string mobType; //type of the mobility module
        string nicType; //type of the NIC module
        @display("bgb=,,white;i=device/laptop;b=40,40,rect");
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
        nic: <nicType> like IWirelessNic {
            parameters:
                @display("p=60,166;i=block/ifcard");
        }
    connections:
        nic.upperLayerOut --> net.lowerLayerIn;
        nic.upperLayerIn <-- net.lowerLayerOut;
        // nic.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> net.lowerControlIn;
        // nic.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- net.lowerControlOut;

        net.upperLayerOut --> appl.lowerLayerIn;
        net.upperLayerIn <-- appl.lowerLayerOut;
        net.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.lowerControlIn;
        net.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;

}

