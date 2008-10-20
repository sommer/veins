package org.mixim.examples.locNetwork;

import org.mixim.base.localization.BaseLocalization;
import org.mixim.base.modules.BaseApplLayer;
import org.mixim.base.modules.BaseArp;
import org.mixim.base.modules.BaseBattery;
import org.mixim.base.modules.BaseNic;
import org.mixim.base.modules.BaseUtility;
import org.mixim.modules.mobility.ConstSpeedMobility;
import org.mixim.modules.netw.QueuedRouting;


module LocNode
{
    parameters:
        string applType; // type of the application layer
        string locType; // type of the localization layer
        @display("bgb=,,white,,");
    gates:
        input radioIn; // gate for sendDirect
    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=130,38,rect;b=24,24,,black,,");
        }
        arp: BaseArp {
            parameters:
                @display("p=130,84,rect;b=24,24,,blue,,");
        }
        battery: BaseBattery {
            parameters:
                @display("p=130,128,rect;b=24,24,,red,,");
        }
        mobility: ConstSpeedMobility {// BaseMobility;
            parameters:
                @display("p=130,172;i=cogwheel2");
        }
        appl: <applType> like BaseApplLayer {
            parameters:
                @display("p=60,50;i=app");
        }
        net: QueuedRouting {
            parameters:
                @display("p=60,108;i=prot1");
        }
        loc: <locType> like BaseLocalization {
            parameters:
                @display("p=60,137");
        }
        nic: BaseNic {
            parameters:
                @display("p=60,166;i=iface");
        }
    connections:
        nic.upperGateOut --> net.lowerGateIn;
        nic.upperGateIn <-- net.lowerGateOut;
        nic.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> net.lowerControlIn;
        nic.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- net.lowerControlOut;

        net.upperGateOut --> loc.lowerGateIn;
        net.upperGateIn <-- loc.lowerGateOut;
        net.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> loc.lowerControlIn;
        net.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- loc.lowerControlOut;

        loc.upperGateOut --> appl.lowerGateIn;
        loc.upperGateIn <-- appl.lowerGateOut;
        loc.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.lowerControlIn;
        loc.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;

}

