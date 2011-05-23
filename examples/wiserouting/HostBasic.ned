package org.mixim.examples.wiserouting;

import org.mixim.base.modules.*;
import org.mixim.modules.nic.INic;
import org.mixim.modules.netw.ArpHost;

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
        @display("bgb=,,white;i=device/laptop");
    gates:
        input radioIn; // gate for sendDirect
    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=130,38;b=24,24,rect,black;i=block/blackboard");
        }
        arp: ArpHost {
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
        nic: <nicType> like INic {
            parameters:
                @display("p=60,166;i=block/ifcard");
        }
    connections:
        nic.upperGateOut --> net.lowerGateIn;
        nic.upperGateIn <-- net.lowerGateOut;
        nic.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> net.lowerControlIn;
        nic.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- net.lowerControlOut;

        net.upperGateOut --> appl.lowerGateIn;
        net.upperGateIn <-- appl.lowerGateOut;
        net.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.lowerControlIn;
        net.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;

}

