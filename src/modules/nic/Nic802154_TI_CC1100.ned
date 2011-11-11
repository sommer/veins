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

//
// This NIC implements a TI CC1100 network interface card
// using the CSMA protocol as specified in IEEE 802.15.4-2006.
//
// Note: To be able to use this Nic in your simulation you have to copy the file 
// "Nic802154_TI_CC2420_Decider.xml" from "modules/nic/" to your simulation directory.
// @author Jerome Rousselot
//
module Nic802154_TI_CC1100 extends WirelessNicBattery
{
    parameters:
        macType = default("CSMA802154");
        
        // power consumption from boards (at 3V)
        sleepCurrent         = 0.02mA; // 20 µA
        rxCurrent            = 16.4 mA;
        decodingCurrentDelta = 0 mA;
        txCurrent            = 17 mA;
        setupRxCurrent       = 8.2mA;
        setupTxCurrent       = 8.2mA;
        rxTxCurrent          = 17 mA;
        txRxCurrent          = 17 mA;

        phy.decider       = xmldoc("Nic802154_TI_CC1100_Decider.xml");
        phy.analogueModels= default(xmldoc("config.xml"));
        //publishRSSIAlways = false;
        phy.sensitivity   = -100 dBm;
        phy.headerLength  = 48 bit; // ieee 802.15.4
        phy.thermalNoise  = -110 dBm;
        phy.maxTXPower    = 1 mW;
        phy.initialRadioState = 0;
        // From TI CC1100 datasheet rev. C
        phy.timeSleepToRX = 0.00108 s;
        phy.timeSleepToTX = 0.00108 s;
        phy.timeRXToTX    = 0.000010 s;
        phy.timeTXToRX    = 0.000022 s;
        phy.timeRXToSleep = 0 s;
        phy.timeTXToSleep = 0 s;

        mac.rxSetupTime   = 0.00108 s;
}

