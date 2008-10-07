import PhyMacNic;
import BaseMobility;
import BaseUtility;


module PhyMacHost
{
    parameters:
        double numHosts; // total number of hosts in the network

        @display("bgb=180,200,white,,;bgp=10,10");
    gates:
        input radioIn;

    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=130,60;i=block/blackboard");
        }
        mobility: BaseMobility {
            parameters:
                @display("p=130,130;i=block/cogwheel");
        }
        normalNic: PhyMacNic {
            parameters:
                @display("b=32,30;p=60,166;i=block/ifcard");
        }
    connections:

        radioIn --> normalNic.radioIn;

}

