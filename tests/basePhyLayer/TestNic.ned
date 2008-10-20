package org.mixim.tests.basePhyLayer;

module TestNic
{
    parameters:
        string connectionManagerName; //name of the ConnectionManager module

        @display("bgb=101,180,white,,;bgp=10,10");
    gates:
        input radioIn; // radioIn gate for sendDirect

    submodules:
        phy: TestPhyLayer {
            parameters:
                @display("p=60,150;i=prot3");

        }
        mac: TestMacLayer;
    connections:
        phy.upperGateOut --> mac.lowerGateIn;
        phy.upperGateIn <-- mac.lowerGateOut;
        phy.upperControlOut --> { @display("ls=red;m=m,50,50,75,0"); } --> mac.lowerControlIn;
        phy.upperControlIn <-- { @display("ls=red;m=m,25,0,25,0"); } <-- mac.lowerControlOut;

        radioIn --> phy.radioIn;

}

