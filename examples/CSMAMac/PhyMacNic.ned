
package org.mixim.examples.CSMAMac;

import org.mixim.modules.phy.PhyLayer;
import org.mixim.modules.mac.CSMAMacLayer;


module PhyMacNic
{
    parameters:
        int id;
        string connectionManagerName; 	//name of the ConnectionManager module

        @display("bgb=101,200,white,,;bgp=10,10");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input upperControlIn;
        output upperControlOut;
        input radioIn; // radioIn gate for sendDirect

    submodules:
        phy: PhyLayer {
            parameters:
                @display("p=60,170;b=80,30,rect,white,,");
        }
        mac: CSMAMacLayer {
            parameters:
                @display("p=60,100;b=80,30,rect,white,,");
        }

    connections:
        mac.upperLayerOut --> { @display("ls=black;m=m,25,50,25,0"); } --> upperLayerOut;
        mac.upperLayerIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- upperLayerIn;
        mac.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> upperControlOut;
        mac.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- upperControlIn;

        phy.upperLayerOut --> { @display("ls=black;m=m,25,50,25,0"); } --> mac.lowerLayerIn;
        phy.upperLayerIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- mac.lowerLayerOut;
        phy.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> mac.lowerControlIn;
        phy.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- mac.lowerControlOut;

        radioIn --> phy.radioIn;

}

