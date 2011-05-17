//***************************************************************************
// * file:        Nic802154_TI_CC2420.ned
// *
// * author:      Jerome Rousselot, Marc Loebbers
// *
// * copyright:   (C) 2008-2010 CSEM SA, Neuchatel, Switzerland.
// *		  	  (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
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
// This NIC implements a Texas Instruments CC 2420 802.15.4 network interface card
// using the CSMA protocol as specified in IEEE 802.15.4-2006.
//
//Note: To be able to use this Nic in your simulation you have to copy the file 
//"Nic802154_TI_CC2420_Decider.xml" from "modules/nic/" to your simulation directory.
// @author Jerome Rousselot
//
module Nic802154_TI_CC2420 like INic
{
    parameters:
        string connectionManagerName = default("");   //name of the ConnectionManager module
        // power consumption from boards (at 3.3V)
	    double sleepCurrent @unit(mA) = 0.000021mA; // 0.021 µA
		double rxCurrent @unit(mA) = 18.8 mA;
		double decodingCurrentDelta @unit(mA) = 0 mA;
		double txCurrent @unit(mA) = 17.4 mA;
		double setupRxCurrent @unit(mA) = 0.6391mA; // 0.002109 W = (ESetupPower+ESetupXtal+ETxCalibrate)/TSetupTx
		double setupTxCurrent @unit(mA) = 0.6845mA; // 0.002259 W = (ESetupPower+ESetupXtal+ERxCalibrate)/TSetupRx
		double rxTxCurrent @unit(mA) = 18.8 mA; // Upper bound
		double txRxCurrent @unit(mA) = 18.8 mA; // idem
		
    gates:
        input upperGateIn; // from upper layers
        input radioIn; // to receive AirFrames
        output upperGateOut; // to upper layers
        output upperControlOut; // control connection
        input upperControlIn;

    submodules:
        phy: PhyLayerBattery {
            parameters:
                decider = xmldoc("Nic802154_TI_CC2420_Decider.xml");
                //publishRSSIAlways = false;
                headerLength = 48 bit; // ieee 802.15.4                
                thermalNoise = -110 dBm;
                // From TI CC1100 datasheet rev. C
                timeSleepToRX = 0.001792 s;
                timeSleepToTX = 0.001792 s;
                timeRXToTX = 0.000192 s;
                timeTXToRX = 0.000192 s;
                timeRXToSleep = 0 s;
                timeTXToSleep = 0 s;
                // power consumption from boards
//                sleepPower = 0.0000000693 W; // 0.021 microA * 3.3 V
//                rxPower = 0.06204 W;   // 18.8 mA * 3.3 V
//                txPower = 0.05742 W;   // 17.4 mA * 3.3 V
//                rxTxPower = 0.06204 W; // Upper bound : 18.8 mA * 3.3 V
//                txRxPower = 0.06204 W; // idem
//                setupTxPower = 0.002109 W; // (ESetupPower+ESetupXtal+ETxCalibrate)/TSetupTx
//                setupRxPower = 0.002259 W; // (ESetupPower+ESetupXtal+ERxCalibrate)/TSetupRx
                @display("p=96,236;i=block/wrxtx");

        }
        mac: CSMA802154 {
            parameters:
                rxSetupTime = 0.001792 s;
                txPower = default(1 mW);
				
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

