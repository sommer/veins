package org.mixim.modules.nic;

import org.mixim.modules.phy.PhyLayerUWBIR;
import org.mixim.modules.mac.AlohaMacLayer;

module Nic802154A
{
    parameters:
        string connectionManagerName; 	//name of the ConnectionManager module
        // power consumption from boards (at 3.3V)
	    double sleepCurrent @unit(mA) = .00303 mA; // 0.021 µA
		double rxCurrent @unit(mA) = 9.1 mA;
		double syncCurrent @unit(mA) = 30.3 mA;
		double txCurrent @unit(mA) = 0.303 mA;
		double setupRxCurrent @unit(mA) = 9.1 mA;
		double setupTxCurrent @unit(mA) = 0.303 mA;
		double rxTxCurrent @unit(mA) = 9.1 mA; 
		double txRxCurrent @unit(mA) = 9.1 mA; 
        

        @display("bgb=101,150,white,,;bgp=10,10");
    gates:
        input radioIn; // radioIn gate for sendDirect
        input upperGateIn; // uppergateIn for incoming messages from network layer
        input upperGateCtrlIn; // uppergateCtrlIn for incoming control messages from network layer
        output upperGateOut; // uppergateOut to send data messages to network layer
        output upperGateCtrlOut; // uppergateCtrlOut to send control messages to network layer

    submodules:
        phy: PhyLayerUWBIR {
            parameters:
                @display("p=60,120;b=80,30,rect,white,,");
            //display: "p=60,150;i=prot3";

        }
        mac: AlohaMacLayer {
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

        upperGateIn --> mac.upperGateIn;
        upperGateCtrlIn --> mac.upperControlIn;
        upperGateOut <-- mac.upperGateOut;
        upperGateCtrlOut <-- mac.upperControlOut;

}

