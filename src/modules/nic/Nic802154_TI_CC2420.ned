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

//
// This NIC implements a Texas Instruments CC 2420 802.15.4 network interface card
// using the CSMA protocol as specified in IEEE 802.15.4-2006.
//
//Note: To be able to use this Nic in your simulation you have to copy the file 
//"Nic802154_TI_CC2420_Decider.xml" from "modules/nic/" to your simulation directory.
// @author Jerome Rousselot
//
module Nic802154_TI_CC2420 extends WirelessNicBattery
{
    parameters:
        macType = default("CSMA802154");
        
        // power consumption from boards (at 3.3V)
        sleepCurrent         = 0.000021mA; // 0.021 µA
        rxCurrent            = 18.8 mA;
        decodingCurrentDelta = 0 mA;
        txCurrent            = 17.4 mA;
        setupRxCurrent       = 0.6391mA; // 0.002109 W = (ESetupPower+ESetupXtal+ETxCalibrate)/TSetupTx
        setupTxCurrent       = 0.6845mA; // 0.002259 W = (ESetupPower+ESetupXtal+ERxCalibrate)/TSetupRx
        rxTxCurrent          = 18.8 mA; // Upper bound
        txRxCurrent          = 18.8 mA; // idem

        phy.decider = xmldoc("Nic802154_TI_CC2420_Decider.xml");
        //publishRSSIAlways = false;
        phy.headerLength = 48 bit; // ieee 802.15.4                
        phy.thermalNoise = -110 dBm;
        // From TI CC1100 datasheet rev. C
        phy.timeSleepToRX = 0.001792 s;
        phy.timeSleepToTX = 0.001792 s;
        phy.timeRXToTX = 0.000192 s;
        phy.timeTXToRX = 0.000192 s;
        phy.timeRXToSleep = 0 s;
        phy.timeTXToSleep = 0 s;

        mac.rxSetupTime = 0.001792 s;
        mac.txPower = default(1 mW);
}

