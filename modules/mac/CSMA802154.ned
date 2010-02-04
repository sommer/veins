//***************************************************************************
//* file:        CSMA802154.ned
//*
//* author:      Jerome Rousselot, Marc Loebbers
//* 
//* copyright:   (C) 2007-2009 CSEM SA, Neuchatel, Switzerland.
//*		 		 (C) 2004 Telecommunication Networks Group (TKN) at
//*              Technische Universitaet Berlin, Germany.
//*
//*              This program is free software; you can redistribute it
//*              and/or modify it under the terms of the GNU General Public
//*              License as published by the Free Software Foundation; either
//*              version 2 of the License, or (at your option) any later
//*              version.
//*              For further information see file COPYING
//*              in the top level directory
//*
//* Funding: This work was partially financed by the European Commission under the  
//* Framework 6 IST Project "Wirelessly Accessible Sensor Populations"  
//* (WASP) under contract IST-034963.
//***************************************************************************
//* part of:    Modifications to the MF-2 framework by CSEM
//**************************************************************************/
package org.mixim.modules.mac;

//
// IEEE 802.15.4-2006 non-beacon enabled CSMA protocol
//
simple CSMA802154 extends csma
{
    parameters:
        @class(csma);
        
        sifs @unit(s) = default(0.000192 s); // 12 symbols
        headerLength @unit(bit) = default(72 bit);
        queueLength = default(100);
        bitrate @unit(bps) = default(250000 bps);
        ackLength @unit(bit) = default(40 bit);        
        macMaxCSMABackoffs = default(5);
        macMaxFrameRetries = default(3);        
        macAckWaitDuration @unit(s) = default(0.00056 s);  // 1+12+10+12 symbols
        ccaDetectionTime @unit(s) = default(0.000128 s); // 8 symbols
        aTurnaroundTime @unit(s) = default(0.000192 s);    // 12 symbols
        //txPower @unit(mW);
        useMACAcks = default(true);
        
        backoffMethod = default("exponential");
        aUnitBackoffPeriod @unit(s) = default(0.00032 s);
        macMinBE = default(3);
        macMaxBE = default(8);
}
