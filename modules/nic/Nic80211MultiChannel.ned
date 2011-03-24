//***************************************************************************
// * file:        Nic80211.ned
// *
// * author:      Marc Loebbers
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *
// *              This program is free software; you can redistribute it
// *              and/or modify it under the terms of the GNU General Public
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later
// *              version.
// *              For further information see file COPYING
// *              in the top level directory
// ***************************************************************************
// * part of:     framework implementation developed by tkn
// **************************************************************************/
package org.mixim.modules.nic;

import org.mixim.modules.mac.Mac80211MultiChannel;
import org.mixim.modules.phy.PhyLayerDetailed;

//
// This NIC implements an 802.11 network interface card with multi channel 
// support and power consumption.
//
// @see Mac80211MultiChannel, Decider80211MultiChannel
// @author Karl Wessel
//
module Nic80211MultiChannel like INic
{
    parameters:
        //name of the ConnectionManager module
        string connectionManagerName = default("");   
    	@display("i=block/ifcard");
    	double sleepCurrent @unit(mA);
        double rxCurrent @unit(mA);
        double decodingCurrentDelta @unit(mA);
        double txCurrent @unit(mA);
        double setupRxCurrent @unit(mA);
        double setupTxCurrent @unit(mA);
        double rxTxCurrent @unit(mA);
        double txRxCurrent @unit(mA);
        double onCurrent @unit(mA) = default(0);
    
    gates:
        input upperGateIn; // to upper layers
        output upperGateOut; // from upper layers
        output upperControlOut; // control information 
        input upperControlIn; // control information 
        input radioIn; // radioIn gate for sendDirect

    submodules:
        mac: Mac80211MultiChannel {
            queueLength = default(20);
            bitrate = default(2E+6bps);            
            autoBitrate = default(false);
            // values if no fading is modelled, gives at most 1% packet error rate
			snr2Mbit = default(1.46dB); 
			snr5Mbit = default(2.6dB);
			snr11Mbit = default(5.68dB);
			neighborhoodCacheSize = default(30);
			neighborhoodCacheMaxAge = default(100s);
			txPower = default(110.11mW);
			defaultChannel = default(1); 
			centerFrequency = default(2.412e9Hz);
            
            @display("p=96,87;i=block/layer");
        }

        phy: PhyLayerDetailed {
            headerLength = 192bit;
            sensitivity = default(-119.5dBm);
            decider = default(xmldoc("Nic80211MultiChannel_Decider.xml"));
            nbRadioChannels = default(15);
            initialRadioChannel = default(1);
            @display("p=96,158;i=block/process;is=n");
        }

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

