package org.mixim.examples.baseNetwork;

import org.mixim.base.modules.*;

module BaseNode
{
    parameters:
        string applType; //type of the application layer
        string netwType; //type of the network layer
        string mobType; //type of the mobility module
        @display("bgb=,,white,,");
    gates:
        input radioIn; // gate for sendDirect
    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=130,38;b=24,24,rect,black;i=block/blackboard");
        }
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
        nic: BaseNic {
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

