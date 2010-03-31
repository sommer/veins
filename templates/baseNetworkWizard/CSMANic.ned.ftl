<#if protocolName=="CSMA">
<#if nedPackageName!="">package ${nedPackageName};</#if>

import org.mixim.modules.mac.CSMAMacLayer;
import org.mixim.modules.phy.PhyLayer;

module CSMANic 
{
    gates:
        input upperGateIn; // to upper layers
        output upperGateOut; // from upper layers
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
        mac.upperGateOut --> { @display("ls=black;m=m,25,50,25,0"); } --> upperGateOut;
        mac.upperGateIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- upperGateIn;
        mac.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> upperControlOut;
        mac.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- upperControlIn;

        phy.upperGateOut --> { @display("ls=black;m=m,25,50,25,0"); } --> mac.lowerGateIn;
        phy.upperGateIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- mac.lowerGateOut;
        phy.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> mac.lowerControlIn;
        phy.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- mac.lowerControlOut;

        radioIn --> phy.radioIn;
}
</#if>

