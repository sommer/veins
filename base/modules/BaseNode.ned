import BaseApplLayer;
import BaseNetwLayer;
import BaseNic;
import BaseMobility;
import BaseArp;
import BaseBattery;
import BaseUtility;

module BaseNode
{
    parameters:
        string applType; //type of the application layer
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
        mobility: BaseMobility {
            parameters:
                @display("p=130,172;i=cogwheel2");
        }
        appl: <applType> like BaseApplLayer {
            parameters:
                @display("p=60,50;i=app");
        }
        net: BaseNetwLayer {
            parameters:
                @display("p=60,108;i=prot1");
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

        net.uppergateOut --> appl.lowergateIn;
        net.uppergateIn <-- appl.lowergateOut;
        net.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.lowerControlIn;
        net.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;

}

