import BaseApplLayer;
//    "BaseNetwLayer",
import QueuedRouting;
import BaseLocalization;
import BaseNic;
//    "BaseMobility",
import ConstSpeedMobility;
import BaseArp;
import BaseBattery;
import BaseUtility;

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
        nic.uppergateOut --> net.lowergateIn;
        nic.uppergateIn <-- net.lowergateOut;
        nic.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> net.lowerControlIn;
        nic.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- net.lowerControlOut;

        net.uppergateOut --> loc.lowergateIn;
        net.uppergateIn <-- loc.lowergateOut;
        net.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> loc.lowerControlIn;
        net.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- loc.lowerControlOut;

        loc.uppergateOut --> appl.lowergateIn;
        loc.uppergateIn <-- appl.lowergateOut;
        loc.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.lowerControlIn;
        loc.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;

}

