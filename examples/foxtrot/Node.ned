import BaseApplLayer;
import BaseAggLayer;
import BaseNetwLayer;
import Nic;
import BaseMobility;
import BaseArp;
import BaseBattery;
import BaseUtility;

module Node
{
    parameters:
        bool isSink;
        string routingType; //type of the network layer
        string aggType; //type of the aggregation layer
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
        agg: <aggType> like BaseAggLayer {
            parameters:
                @display("p=60,50;i=agg");
        }
        net: <routingType> like BaseNetwLayer {
            parameters:
                @display("p=60,108;i=prot1");
        }
        nic: Nic {
            parameters:
                @display("p=60,166;i=iface");
        }
    connections:
        nic.uppergateOut --> net.lowergateIn;
        nic.uppergateIn <-- net.lowergateOut;
        nic.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> net.lowerControlIn;
        nic.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- net.lowerControlOut;

        net.uppergateOut --> agg.lowergateIn;
        net.uppergateIn <-- agg.lowergateOut;
        net.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> agg.lowerControlIn;
        net.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- agg.lowerControlOut;

        agg.uppergateOut --> appl.lowergateIn;
        agg.uppergateIn <-- appl.lowergateOut;
        agg.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.lowerControlIn;
        agg.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.lowerControlOut;
        radioIn --> nic.radioIn;
}

