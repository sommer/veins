//***************************************************************************
// * file:        Nic802154_TI_CC1100.ned
// *
// * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
// *
// * copyright:   (C) 2008-2010 CSEM SA, Neuchatel, Switzerland.
// *
// *              This program is free software; you can redistribute it
// *              and/or modify it under the terms of the GNU General Public
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later
// *              version.
// *              For further information see file COPYING
// *              in the top level directory
// *
// * Funding: This work was partially financed by the European Commission under the  
// * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"  
// * (WASP) under contract IST-034963.
// **************************************************************************/
package org.mixim.modules.nic;

import org.mixim.modules.phy.PhyLayerBattery;
import org.mixim.modules.mac.CSMA802154;


//
// This NIC implements a TI CC1100 network interface card
// using the CSMA protocol as specified in IEEE 802.15.4-2006.
//
// Note: To be able to use this Nic in your simulation you have to copy the file 
// "Nic802154_TI_CC2420_Decider.xml" from "modules/nic/" to your simulation directory.
// @author Jerome Rousselot
//
module Nic802154_TI_CC1100 like INic
{
	parameters:
        string connectionManagerName = default("");   //name of the ConnectionManager module
	    // power consumption from boards (at 3V)
	    double sleepCurrent @unit(mA) = 0.02mA; // 20 µA
		double rxCurrent @unit(mA) = 16.4 mA;
		double decodingCurrentDelta @unit(mA) = 0 mA;
		double txCurrent @unit(mA) = 17 mA;
		double setupRxCurrent @unit(mA) = 8.2mA;
		double setupTxCurrent @unit(mA) = 8.2mA;
		double rxTxCurrent @unit(mA) = 17 mA;
		double txRxCurrent @unit(mA) = 17 mA;
	
    gates:
        input upperGateIn; // from upper layers
        input radioIn; // to receive AirFrames
        output upperGateOut; // to upper layers
        output upperControlOut; // control connection
        input upperControlIn;

    submodules:
        phy: PhyLayerBattery {
            parameters:
                decider = xmldoc("Nic802154_TI_CC1100_Decider.xml");
				analogueModels = default(xmldoc("config.xml"));
                //publishRSSIAlways = false;
				sensitivity = -100 dBm;
                headerLength = 48 bit; // ieee 802.15.4
                thermalNoise = -110 dBm;
                maxTXPower = 1 mW;
                initialRadioState = 0;
                // From TI CC1100 datasheet rev. C
                timeSleepToRX = 0.00108 s;
                timeSleepToTX = 0.00108 s;
                timeRXToTX = 0.000010 s;
                timeTXToRX = 0.000022 s;
                timeRXToSleep = 0 s;
                timeTXToSleep = 0 s;
                @display("p=96,236;i=block/wrxtx");

        }
        mac: CSMA802154 {
            parameters:
                rxSetupTime = 0.00108 s;
				
                @display("p=96,87;i=block/layer");

        }
        
    connections:

        radioIn --> phy.radioIn;

        phy.upperGateIn <-- { @display("m=m,25,0,25,0"); } <-- mac.lowerGateOut;
        phy.upperGateOut --> { @display("m=m,73,0,50,50"); } --> mac.lowerGateIn;
        phy.upperControlOut --> mac.lowerControlIn;
        phy.upperControlIn <-- mac.lowerControlOut;

        mac.upperGateOut --> upperGateOut;
        mac.upperControlOut --> upperControlOut;
        mac.upperGateIn <-- upperGateIn;
		mac.upperControlIn <-- upperControlIn;
}

