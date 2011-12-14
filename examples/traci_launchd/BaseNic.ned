package org.mixim.examples.traci_launchd;

import org.mixim.modules.mac.CSMAMacLayer;
import org.mixim.modules.phy.PhyLayer;

//
// This NIC implements an 802.11 network interface card.
//
// @see Mac80211, Decider80211
// @author Marc Loebbers
//
module BaseNic 
{
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        output upperControlOut; // control information 
        input upperControlIn; // control information 
		input radioIn; // radioIn gate for sendDirect

    submodules:
        mac: CSMAMacLayer {
            @display("p=96,87;i=block/layer");
        }
        
        phy: PhyLayer {
            @display("p=106,157;i=block/process_s");
        }
		//radio: SingleChannelRadio;
	    // display: "p=200,30;b=30,25";

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

