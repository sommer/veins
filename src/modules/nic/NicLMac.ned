//
//  LMAC for MF 2.02, omnetpp 3.4
//
//  Created by Anna Foerster on 10/10/08.
//  Copyright 2008 Universita della Svizzera Italiana. All rights reserved.
//
//  Coverted to OMNeT++ 4 by Rudolf Hornig
//
//  Converted to MiXiM by Kapourniotis Theodoros
//
//

package org.mixim.modules.nic;

import org.mixim.modules.mac.LMacLayer;
import org.mixim.modules.phy.PhyLayerBattery;

//
// This module implements a LMAC network interface card using
// the "LMacLayer" MAC layer module
//
module NicLMAC like IWirelessNic
{
    parameters:
        double sleepCurrent @unit(mA); // 20 µA
		double rxCurrent @unit(mA);
		double decodingCurrentDelta @unit(mA);
		double txCurrent @unit(mA);
		double setupRxCurrent @unit(mA);
		double setupTxCurrent @unit(mA);
		double rxTxCurrent @unit(mA);
		double txRxCurrent @unit(mA);
		
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        output upperControlOut; // control information 
        input upperControlIn; // control information 
		input radioIn; // radioIn gate for sendDirect
    submodules:
        mac: LMacLayer {
            parameters:
                @display("p=96,87;i=block/layer");
        }
        
        phy: PhyLayerBattery {
            @display("p=106,157;i=block/process_s");
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
