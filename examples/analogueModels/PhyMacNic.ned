import TestPhyLayer;
import SimpleMacLayer;

module PhyMacNic
{
    parameters:
        string connectionManagerName; 	//name of the ConnectionManager module

        @display("bgb=101,150,white,,;bgp=10,10");
    gates:
        input radioIn; // radioIn gate for sendDirect

    submodules:
        phy: TestPhyLayer {
            parameters:
                @display("p=60,120;b=80,30,rect,white,,");
            //display: "p=60,150;i=prot3";

        }
        mac: SimpleMacLayer {
            parameters:
                @display("p=60,45;b=80,30,rect,white,,");
        	//display: "p=60,70;i=prot3";

        }
    connections:
        phy.upperGateOut --> { @display("ls=black;m=m,25,50,25,0"); } --> mac.lowerGateIn;
        phy.upperGateIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- mac.lowerGateOut;
        phy.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> mac.lowerControlIn;
        phy.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- mac.lowerControlOut;

        radioIn --> phy.radioIn;

}

