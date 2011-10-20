package org.mixim.examples.analogueModels;

module PhyMacNic
{
    parameters:
        string connectionManagerName; 	//name of the ConnectionManager module

        @display("bgb=101,150,white,,;bgp=10,10");
    gates:
        input radioIn; // radioIn gate for sendDirect

    submodules:
        phy: SamplePhyLayer {
            parameters:
                @display("p=60,120;b=80,30,rect,white,,");

        }
        mac: SimpleMacLayer {
            parameters:
                @display("p=60,45;b=80,30,rect,white,,");

        }
    connections:
        phy.upperLayerOut --> { @display("ls=black;m=m,25,50,25,0"); } --> mac.lowerLayerIn;
        phy.upperLayerIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- mac.lowerLayerOut;
        phy.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> mac.lowerControlIn;
        phy.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- mac.lowerControlOut;

        radioIn --> phy.radioIn;

}

